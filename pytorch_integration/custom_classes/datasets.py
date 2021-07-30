from PIL import Image
from typing import Any, Callable, cast, Dict, List, Optional, Tuple
from torchvision.datasets import VisionDataset

import prisma_binding as prisma 
import shuffle_filenames
import os
import io
import torch
import os.path


def has_file_allowed_extension(filename: str, extensions: Tuple[str, ...]) -> bool:
    return filename.lower().endswith(extensions)


def is_image_file(filename: str) -> bool:
    return has_file_allowed_extension(filename, IMG_EXTENSIONS)


def make_dataset(
        directory: str,
        class_to_idx: Dict[str, int],
        extensions: Optional[Tuple[str, ...]] = None,
        is_valid_file: Optional[Callable[[str], bool]] = None
) -> Dict[int, Tuple[str, int]]:
    instances = {}
    directory = os.path.expanduser(directory)
    both_none = extensions is None and is_valid_file is None
    both_something = extensions is not None and is_valid_file is not None
    if both_none or both_something:
        raise ValueError("Both extensions and is_valid_file cannot be None or not None at the same time")
    if extensions is not None:
        def is_valid_file(x: str) -> bool:
            return has_file_allowed_extension(x, cast(Tuple[str, ...], extensions))
    is_valid_file = cast(Callable[[str], bool], is_valid_file)
    index = 0
    for target_class in sorted(class_to_idx.keys()):
        class_index = class_to_idx[target_class]
        target_dir = os.path.join(directory, target_class)
        if not os.path.isdir(target_dir):
            continue
        for root, _, fnames in sorted(os.walk(target_dir, followlinks=True)):
            for fname in fnames:
                path = os.path.join(root, fname)
                if is_valid_file(path):
                    size = os.stat(path).st_size
                    item = size, path, class_index
                    instances[index] = item
                index += 1
    return instances

def tensor_loader(path: str):
    return torch.load(path)

def tensor_prisma_loader(prisma: prisma.PrismaClient, path: str, size: int):
    status, byts = prisma.read(path, size)
    return torch.load(io.BytesIO(byts))

def pil_prisma_loader(prisma: prisma.PrismaClient, path: str, size: int) -> Image.Image:
    status, byts = prisma.read(path, size)
    img = Image.open(io.BytesIO(byts))
    return img.convert('RGB')

def pil_loader(path: str) -> Image.Image:
    with open(path, 'rb') as f:
        img = Image.open(f)
        return img.convert('RGB')

IMG_EXTENSIONS = ('.jpg', '.jpeg', '.png', '.ppm', '.bmp', '.pgm', '.tif', '.tiff', '.webp', '.pt')


class EpochShuffleImageFolder(VisionDataset):
    def __init__(
            self,
            root: str,
            epochs: int,
            tensor_data: bool,
            multiprocessing: bool,
            multigpu: bool,
            extensions: Optional[Tuple[str, ...]] = IMG_EXTENSIONS,
            transform: Optional[Callable] = None,
            target_transform: Optional[Callable] = None,
            is_valid_file: Optional[Callable[[str], bool]] = None,
    ):
        super(EpochShuffleImageFolder, self).__init__(root, transform=transform, target_transform=None)

        classes, class_to_idx = self._find_classes(self.root)
        samples = make_dataset(self.root, class_to_idx, extensions, is_valid_file)
        shuffled_samples_indexes = shuffle_filenames.shuffle_indexes(samples, epochs)

        if len(samples) == 0:
            msg = "Found 0 files in subfolders of: {}\n".format(self.root)
            if extensions is not None:
                msg += "Supported extensions are: {}".format(",".join(extensions))
            raise RuntimeError(msg)

        if tensor_data:
            self.loader=tensor_prisma_loader
        else:
            self.loader=pil_prisma_loader
            
        self.extensions = extensions

        self.epochs = epochs
        self.classes = classes
        self.class_to_idx = class_to_idx
        self.samples = samples

        if multiprocessing:
            self.prisma = None
        elif multigpu:
            self.prisma = prisma.PrismaClient()
        else:
            self.prisma = prisma.Prisma()
        self.shuffled_samples_indexes = shuffled_samples_indexes

    def _find_classes(self, dir: str) -> Tuple[List[str], Dict[str, int]]:
        classes = [d.name for d in os.scandir(dir) if d.is_dir()]
        classes.sort()
        class_to_idx = {cls_name: i for i, cls_name in enumerate(classes)}
        return classes, class_to_idx

    def __getitem__(self, index: int) -> Tuple[Any, Any]:
        size, path, target = self.samples[index]
        sample = self.loader(self.prisma, path, size)
        if self.transform is not None:
            sample = self.transform(sample)

        return sample, target

    def __len__(self) -> int:
        return len(self.samples)

    def start_client(self):
        self.prisma = prisma.PrismaClient()

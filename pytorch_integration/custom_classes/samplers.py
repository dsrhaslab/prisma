from torch.utils.data.sampler import Sampler
from typing import Sized
import math

from datasets import EpochShuffleImageFolder
from typing import Optional, Iterator
import torch.distributed as dist


class ShuffleSampler(Sampler):

    def __init__(self, data_source: EpochShuffleImageFolder):
        self.data_source = data_source
        self.indexes = data_source.shuffled_samples_indexes
        self.current_epoch = 1

    def __iter__(self):
        res_list = self.indexes[(self.current_epoch - 1) * len(self): self.current_epoch * len(self)]
        self.current_epoch += 1
        return iter(res_list)

    def __len__(self) -> int:
        return len(self.data_source)


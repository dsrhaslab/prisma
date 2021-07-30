# Prisma

Prisma is a storage data plane that accelerates the training performance of Deep Learning (DL) frameworks.
It implements a parallel data prefetching mechanism that reads training data in advance and stores it in an in-memory buffer to serve incoming I/O requests of DL frameworks.

## Build instructions

To build PRISMA, run the following commands:

```
$ cd prisma/build
$ cmake ../
$ make
```

After running the commands, the shared library `libprisma.so` will appear in the `build` directory.


### Dependencies

PRISMA depends on the [Boost C++ libraries](https://www.boost.org/), so please install them before running the build commands.

## Use instructions

### Create filenames list

Before executing PRISMA, a `filenames_list` must be created, so that PRISMA knows the files that must be read in advance, and the order in which they must be cached.

To create the `filenames_list`, use the `shuffle_filenames/shuffle_filenames.py` script, by running the following command:
```
python3 shuffle_filenames.py -d "/path/to/dataset/dir" -e 2 -r "train-[0-9]+.jpg"
```

- `-d`: path to the directory where the dataset files are stored
- `-e`: number of training epochs
- `-r`: regex that corresponds to the filename of the dataset files. If this option is not used, all files in `/path/to/dataset/dir` will be considered.

After running this script, the file `/home_dir/prisma/filenames_list` will be created.

The `shuffled_filenames` module also provides a `shuffle_filenames(files_list, n_epochs)` function that can be called from inside another Python script. This function creates the `filenames_list` with the filenames present in `files_list`, considering that `n_epochs` training epochs will be performed, and returns a list with the content of `filenames_list`.

### Read data

After including `libprisma.so` in your C++ project and creating the `filenames_list`, you simply need to instantiate PRISMA and evoke its `read` method to read the dataset files. Here is a simple example:

```
#include <iostream>
#include "prisma/prisma.h"

int main() {
  // Create PRISMA instance
  Prisma prisma = Prisma();

  std::string filename = "/path/to/file.txt";
  int n = 256;
  off_t offset = 0;
  char res[n];

  // Read 256 bytes from file.txt, starting at offset 0
  // Store result in res
  ssize_t bytes_read = prisma.read(filename, res, n, offset);

  // Print result
  std::cout << res << std::endl;

  return 0;
}
```

For this example to run as expected, the file `/home_dir/prisma/filenames_list` previously created, must contain a single line with `/path/to/file.txt`.

## Configurations
The number of I/O threads and buffer size used by PRISMA can be configured using the `configs` file. This file must be stored in `/home_dir/prisma` when PRISMA is executed.

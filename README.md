# Prisma

Prisma is a storage data plane that accelerates the training performance of Deep Learning (DL) 
frameworks. It implements a parallel data prefetching mechanism that reads training data in advance 
and stores it in an in-memory buffer to serve incoming I/O requests of DL frameworks.

Please cite the following paper if you use Prisma:

**The Case for Storage Optimization Decoupling in Deep Learning Frameworks.**
Ricardo Macedo, Cláudia Correia, Marco Dantas, Cláudia Brito, Weijia Xu, Yusuke Tanimura, Jason Haga, João Paulo.
*IEEE International Conference in Cluster Computing @ 1st Workshop on Re-envisioning Extreme-Scale I/O for Emerging Hybrid HPC Workloads.*

```bibtex
@inproceedings {Macedo2021Prisma,
  title     = {{The Case for Storage Optimization Decoupling in Deep Learning Frameworks}},
  author    = {Ricardo Macedo and Cl{\'{a}}udia Correia and Marco Dantas and Cl{\'{a}}udia Brito and Weijia Xu and Yusuke Tanimura and Jason Haga and Jo{\~{a}}o Paulo},
  booktitle = {{IEEE International Conference on Cluster Computing}},
  pages     = {649--656},
  year      = {2021},
  doi       = {10.1109/Cluster48925.2021.00096},
  publisher = {{IEEE}}
}
```

## Getting started with Prisma
This tutorial will guide you through the installation and configuration of Prisma.

#### **Install dependencies**

Prisma was built, compiled, and tested with `g++-9.3.0` and `cmake-3.16`.
It has dependencies with [Boost C++ libraries](https://www.boost.org/), so please install them 
before running the build commands.
Prisma also has dependencies with [Intel Threading Building Blocks (TBB)](https://github.com/oneapi-src/oneTBB), 
but these are installed (dynamically) at compile time.

```shell
$ wget https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz
$ tar xzvf boost_1_76_0.tar.gz
$ cd boost_1_76_0/
$ ./bootstrap.sh --prefix=/opt 
$ ./b2
$ sudo ./b2 install
$ grep "#define BOOST_LIB_VERSION" /opt/include/boost/version.hpp
```

#### **Install Prisma**
To build Prisma, run the following commands:

```shell
$ git clone https://github.com/dsrhaslab/prisma.git
$ cd prisma/prisma/build
$ cmake ..
$ make
```

After running the commands, the shared library `libprisma.so` will appear in the `build` directory.

#### **Create the filenames list**

Before executing Prisma, a `filenames_list` must be created, so that Prisma knows the files that 
must be read in advance, and the order in which they must be cached.

To create the `filenames_list`, use the `shuffle_filenames/shuffle_filenames.py` script, by running 
the following command:
```shell
python3 shuffle_filenames.py -d "/path/to/dataset/dir" -e 2 -r "train-[0-9]+.jpg"
```

- `-d`: path to the directory where the dataset files are stored
- `-e`: number of training epochs
- `-r`: regex that corresponds to the filename of the dataset files. If this option is not used, all 
- files in `/path/to/dataset/dir` will be considered.

After running this script, the file `/home_dir/prisma/filenames_list` will be created.

The `shuffled_filenames` module also provides a `shuffle_filenames (files_list, n_epochs)` function 
that can be called from inside another Python script. 
This function creates the `filenames_list` with the filenames present in `files_list`, considering 
that `n_epochs` training epochs will be performed, and returns a list with the content of 
`filenames_list`.

#### **Read data**

After including `libprisma.so` in your C++ project and creating the `filenames_list`, you simply 
need to instantiate Prisma and evoke its `read` method to read the dataset files. 
Here is a simple example:

```cpp
#include <iostream>
#include <prisma/prisma.h>

int main() {
  // Create Prisma instance
  Prisma prisma = Prisma();

  std::string filename = "/path/to/file.txt";
  int n = 256;
  off_t offset = 0;
  char res[n];

  // Read 256 bytes from file.txt, starting at offset 0
  // Store result in res
  ssize_t bytes_read = prisma.read (filename, res, n, offset);

  // Print result
  std::cout << res << std::endl;

  return 0;
}
```

For this example to run as expected, the file `/home_dir/prisma/filenames_list` previously created, 
must contain a single line with `/path/to/file.txt`.

#### **Integrating Prisma with Deep Learning frameworks**

**TensorFlow.** 
Please refer to [`prisma/tensorflow_integration`](https://github.com/dsrhaslab/prisma/tree/main/tensorflow_integration)
for details on how to integrate Prisma with TensorFlow.

**PyTorch.**
Please refer to [`prisma/pytorch_integration`](https://github.com/dsrhaslab/prisma/blob/main/pytorch_integration)
for details on how to integrate Prisma with PyTorch.

#### **Configurations**
The number of I/O threads and buffer size used by Prisma can be configured using the `configs` file. 
This file must be stored in `/home_dir/prisma` when Prisma is executed.

## Acknowledgments
>We thank the [National Institute of Advanced Industrial Science and Technologies (AIST)](https://www.aist.go.jp/index_en.html)
for providing access to computational resources of AI Bridging Cloud Infrastructure (ABCI).
>This work was financed by National Funds through the Portuguese Foundation for Science and 
Technology (FCT) on the scope of the UT Austin Portugal Program within project [PAStor](https://pastor-project.github.io)
(UTA-EXPL/CA/0075/2019) and PhD Fellowships SFRH/BD/146059/2019 and SFRH/BD/146528/2019. 

<p align="center">
    <img src=".media/fct-logo.png" width="50">
    <img src=".media/utaustin-portugal-logo.png" width="130">
    <img src=".media/aist-logo.gif" width="130">
</p>

## Contact
Please contact us at `claudia.s.mendonca@inesctec.pt`, `rgmacedo@inesctec.pt` or `jtpaulo@inesctec.pt` 
with any questions.

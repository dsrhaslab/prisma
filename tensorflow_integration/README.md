# Integrating PRISMA with TensorFlow

## Build and install TensorFlow

To build TensorFlow with PRISMA file system implementation, run the following command:

```
$ cd tensorflow
$ ./build_tensorflow.sh
```

After successfully running these commands, TensorFlow package will be in `tensorflow_integration/tensorflow_pkg`.
Then, install the package using this command:

```
$ pip3 install tensorflow_integration/tensorflow_pkg/tensorflow-pkg.whl
```

TensorFlow can now be used as you commonly do, however, you will have the possibility of reading data using PRISMA.

## Use PRISMA with TensorFlow

To read files using PRISMA, simply add `prisma://localhost` to the beggining of each dataset file path and use the `shuffle_filenames` module to create the `filenames_list`.
File `imagenet-preprocessing-example.py` presents an adapted version of the default ImageNet pre-processing script of [CTL implementation for ResNet-50](https://github.com/tensorflow/models/tree/master/official/vision/image_classification/resnet), that uses PRISMA.

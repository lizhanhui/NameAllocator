## Introduction

This project is to allocate broker name conforming to specified patterns.


## How to build

1. Please install Glog following instruction [here](https://github.com/google/glog/blob/master/INSTALL)
1. ZooKeeper C/C++ binding is required and its instruction can be found [here](https://zookeeper.apache.org/doc/r3.1.2/zookeeperProgrammers.html#C+Binding)
1. In the main source folder, execute the following commands to build

    ``sh
    
        mkdir build
        cd build
        cmake ..
        make
        
    ``
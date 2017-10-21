## Introduction

This project is to allocate broker name conforming to specified patterns.


## How to build

1. Please install Glog following instruction [here](https://github.com/google/glog/blob/master/INSTALL)
1. Please install Gflags following instructions [here](https://github.com/gflags/gflags/blob/master/INSTALL.md)
1. Please install Google Tests following instructions [here](https://github.com/google/googletest)
1. Please install Google Benchmark following instructions [here](https://github.com/google/benchmark)
1. ZooKeeper C/C++ binding is required and its instruction can be found [here](https://zookeeper.apache.org/doc/r3.1.2/zookeeperProgrammers.html#C+Binding)
1. In the main source folder, execute the following commands to build

    ```sh
    
        mkdir build
        cd build
        cmake ..
        make
        
    ```

## Broker name allocation strategy

1. If ${HOME}/rmq/conf/broker.conf has a brokerName following pattern `broker\d{1,}`, this application try to register
   this name to zookeeper and output this name if successfully registered; In case the name has been taken, a brand new
   broker name is generated and registered.

2. If broker name in the broker.conf file does not match `broker\d{1, }` pattern, a new name will be registered and returned.

3. The application accepts two optional parameters, span and IP. Span means maximum number of IPs that may share a
   common name, by default, 2;
   IP is the IP address to register, by default, IP of the localhost.

4. ZK connecting string is determined by the following environment variables: `middleware_zk_hosts`, `middleware_zk_port`

5. Application instance competes for creation of /mq/lock node. Once successful, this instance acquires a global lock.
   The lock is released through deleting /mq/lock node in destructor of BrokerNameAllocator class.
   Aka, [RAII](http://en.cppreference.com/w/cpp/language/raii) is employed.
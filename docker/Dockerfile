FROM ubuntu:artful

WORKDIR /root/

USER root

COPY compile.sh /root/

RUN apt-get update && \
    apt-get install build-essential cmake git wget openjdk-8-jdk -y && \
    git clone https://github.com/gflags/gflags.git && \
    git clone https://github.com/google/glog.git && \
    git clone http://gitlab.alibaba-inc.com/messaging/NameAllocator.git && \
    git clone https://github.com/google/googletest.git && \
    git clone https://github.com/google/benchmark.git && \
    wget https://mirrors.tuna.tsinghua.edu.cn/apache/zookeeper/zookeeper-3.4.11/zookeeper-3.4.11.tar.gz && \
    apt-get clean all


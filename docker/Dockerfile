FROM ubuntu:artful

WORKDIR /root/

USER root

COPY compile.sh /root/

RUN apt-get update && \
    apt-get install build-essential cmake git libzookeeper-mt2 libzookeeper-mt-dev vim zookeeper net-tools -y && \
    git clone https://github.com/gflags/gflags.git && \
    git clone https://github.com/google/glog.git && \
    git clone http://gitlab.alibaba-inc.com/messaging/NameAllocator.git && \
    git clone https://github.com/google/googletest.git && \
    git clone https://github.com/google/benchmark.git && \
    apt-get clean all


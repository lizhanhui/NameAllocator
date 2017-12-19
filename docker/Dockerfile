FROM ubuntu:artful

WORKDIR /root/

USER root

COPY compile.sh /root/

RUN apt-get update && \
    apt-get install build-essential cmake git wget openjdk-8-jdk libzookeeper-mt-dev gdb -y && \
    git clone https://github.com/Tencent/rapidjson.git && \
    git clone http://gitlab.alibaba-inc.com/messaging/NameAllocator.git -b develop && \
    wget https://mirrors.tuna.tsinghua.edu.cn/apache/zookeeper/zookeeper-3.4.11/zookeeper-3.4.11.tar.gz && \
    apt-get clean all


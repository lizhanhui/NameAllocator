#!/usr/bin/env bash

cd `dirname $0`
DIR=`pwd`

function build() {
    cd $DIR/$1
    if test ! -d build; then
        mkdir build
    fi
    cd build
    cmake ..
    make
    make install
}

build rapidjson
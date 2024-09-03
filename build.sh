#!/bin/sh

ldd --version 

export CC=arm-linux-gnueabihf-gcc
export CXX=arm-linux-gnueabihf-g++

rm -rf build

cmake -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX} -DBUILD_DIST=ON .

make all install dist

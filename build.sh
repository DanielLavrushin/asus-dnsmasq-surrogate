#!/bin/bash

rm -rf build

cmake .
cmake -DBUILD_DIST=ON ..
make all install dist
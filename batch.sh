#!/bin/bash

SCRIPT=$(readlink -f "$0")
BASEDIR=$(dirname "$SCRIPT")
cd $BASEDIR

if [ ! -d future_net ] || [ ! -f readme.txt ]
then
    echo "ERROR: $BASEDIR is not a valid directory of SDK-gcc for future_net."
    echo "  Please run this script in a regular directory of SDK-gcc."
    exit -1
fi

tmp=$(cmake --version 2>&1)
if [ $? -ne 0 ]
then
    echo "ERROR: You should install cmake(2.8 or later) first."
    echo "  Please run 'sudo at-get install cmake' to install it."
    echo "  or goto https://cmake.org to download and install it."
    exit
fi

rm -fr bin
mkdir bin
rm -fr build
mkdir build
cd build
cmake ../future_net
make 

cd ..
cp -r future_net bin/code
cd bin
tar -zcPf future_net.tar.gz *
cp future_net.tar.gz ../
rm -rf code
rm -f future_net.tar.gz
cd ..
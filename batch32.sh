#!/bin/bash
cd `dirname $0`
rm -f lib/lib_io.a
cp lib/32/lib_io.a lib/
rm -fr bin
mkdir bin
cd build
shopt -s extglob
rm -fr !(Makefile)
make
cd ..
cp -r future_net bin/code
cd bin
tar -zcPf future_net.tar.gz *
cp future_net.tar.gz ../
rm -rf code
rm -f future_net.tar.gz
cd ..

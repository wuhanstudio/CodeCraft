#!/bin/bash
cd `dirname $0`
rm -f lib/lib_io.a
cp lib/64/lib_io.a lib/
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

if [ -e bin/future_net ]
then
	echo --------------------------------------------
	echo 替换测试文件放在test-case topo.csv demand.csv
	echo --------------------------------------------
	read -p "确认开始测试?"

	cp ../test-case/*.csv bin/
	cd bin
	chmod +x ./future_net
	./future_net topo.csv demand.csv result.csv
    
	cp ../validate ./
    	echo --------------------------------------------
	echo 自动查找结果路径是否可行
	echo --------------------------------------------
	read -p "确认开始查找?"    	
    	./validate
else
	echo [错误]没有找到future_net,编译失败
fi

cd ..
	


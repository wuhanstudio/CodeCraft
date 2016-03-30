#!/bin/bash

SCRIPT=$(readlink -f "$0")
BASEDIR=$(dirname "$SCRIPT")

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

if [ -e bin/future_net ]
then
    read -p "请输入批处理文件夹:" caseName
    if [ -e ../succed/$caseName ]
    then 
        chmod -R 777 ../succed/$caseName    
    else
        mkdir ../succed/$caseName
        chmod -R 777 ../succed/$caseName        
    fi
    
	echo --------------------------------------------
	echo 替换测试文件放在test-case topo.csv demand.csv
	echo --------------------------------------------
    for((i=0;i<10;i++));
    do
	    cd bin        
        if [ -e topo.csv ]
        then
            rm *.csv
        fi
        
        if [ -e *result_0.txt ]
        then        
            rm *.txt
        fi
    	read -p "确认开始测试?"
        cd ..
        cp ../test-case/$caseName/case$i/topo.csv  bin/
        cp ../test-case/$caseName/case$i/demand.csv bin/
	    
        cd bin 
	    chmod +x ./future_net
	    ./future_net topo.csv demand.csv result.csv
    
        cp ../validate ./
        if [ -e result_0.txt ]
        then
            cp -r ../../test-case/$caseName/case$i ../../succed/$caseName
            chmod -R 777 ../../succed/$caseName/case$i
            echo --------------------------------------------
	        echo 自动查找结果路径是否可行
	        echo --------------------------------------------
	        read -p "确认开始查找?"    	
            ./validate
        else
             echo ------此case无解------
        fi
             cd ..
        left=`expr 9 - $i`
        read -p "继续下一轮测试? 还剩下$left次"   
    done
        
else
	echo [错误]没有找到future_net,编译失败
fi

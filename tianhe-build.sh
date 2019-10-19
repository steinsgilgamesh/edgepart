#!/bin/bash -e

echo "make sure you are in the build dir."

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/BIGDATA1/buaa_wffan_1/local/lib64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/BIGDATA1/buaa_wffan_1/local/lib
export JAVA_HOME="/BIGDATA1/app/jdk/8u141-b15-gcc-4.8.5"

CXX=g++ CC=gcc cmake .. -DCMAKE_BUILD_TYPE=Release -DTIANHE=true -DBOOST_ROOT="/BIGDATA1/buaa_wffan_1/local"  -Dgflags_DIR=/BIGDATA1/buaa_wffan_1/local -DGFLAGS_INCLUDE_DIR=/BIGDATA1/buaa_wffan_1/local/include -DGFLAGS_LIBRARY=/BIGDATA1/buaa_wffan_1/local/lib/libgflags.so -DGLOG_INCLUDE_DIR=/BIGDATA1/buaa_wffan_1/GLOG/include -DGLOG_LIBRARY=/BIGDATA1/buaa_wffan_1/GLOG/lib/libglog.so 
make -j24


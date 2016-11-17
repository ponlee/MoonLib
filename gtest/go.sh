#!/bin/sh

if [ $# -gt 0 ];then
    FILTER="--gtest_filter=$1"
fi

cd ..
make
cd -
make
./MoonLibTest $FILTER


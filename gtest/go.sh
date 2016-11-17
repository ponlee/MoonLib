#!/bin/sh

#FILTER="--gtest_filter=:CppNet.*"



cd ..
make
cd -
make
./MoonLibTest $FILTER


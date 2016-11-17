#!/bin/sh

#cp ../output/objs/x86_64/debug/zone_svr/zone_svr/*.gcda .
#cp ../output/objs/x86_64/debug/zone_svr/zone_svr/*.gcno .
#gcov *.gcda -b
cd ..
gcov src/*.gcda

#last_line = ""
#cat guild_service_impl.cpp.gcov  | while read line
#do
#   
#   echo "File: ${line}"
#done

cd -
mkdir -p gcov

mv ../*.gcov gcov
rm -rf ../src/*.gcda
rm -rf src/*.gcda

#!/usr/bin/env bash
echo Rebuilding DEBUG mwax_db2fits...
rm *.cmake; rm CMakeCache.txt; rm -rf CMakeFiles; rm MakeFile; rm -rf bin; cmake . -D CMAKE_BUILD_TYPE=Debug && make
echo Success

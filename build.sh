#!/usr/bin/env bash
echo Rebuilding mwax_db2fits...
rm *.cmake; rm CMakeCache.txt; rm -rf CMakeFiles; rm MakeFile; rm -rf bin; cmake CMakeLists.txt && make
echo Success

#!/bin/bash

cmake .
if [[ $? -eq 0 ]]
then
	make
fi

echo "Deleting intermediate files"
rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile libDaemon.a
echo "Files have been deleted"




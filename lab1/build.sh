#!/bin/bash

cmake .
if [[ $? -eq 0 ]]
then
	make
fi

echo "Deleting intermediate files"
rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile libDaemon.a
cd ..
rm -r .vscode
echo "Files have been deleted"




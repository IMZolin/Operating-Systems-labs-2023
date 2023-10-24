#!/bin/bash

set -euo pipefail
pid_file=/run/mydaemon.pid

cmake .
if [[ $? -eq 0 ]]
then
	make
fi

echo "Deleting intermediate files"
rm -rf CMakeFiles cmake_install.cmake CMakeCache.txt Makefile
echo "Files have been deleted"

mv mydaemon mydaemon

sudo touch $pid_file
sudo chmod 666 $pid_file


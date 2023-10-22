#!/bin/bash
pid_file=/run/mydaemon.pid
cmake .
if [[ $? -eq 0 ]]
then
  make
  if [[ $? -eq 0 ]]
  then
    ./main
  fi
fi

echo "Deleting intermediate files"
rm -rf CMakeFiles CMakeCache.txt Makefile main cmake_install.cmake
echo "Files have been deleted"

mv build/mydaemon mydaemon
rm -rf build

sudo touch $pid_file
sudo chmod 666 $pid_file
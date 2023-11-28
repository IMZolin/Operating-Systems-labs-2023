#!/bin/bash
mkdir build
cd build

cmake ..
make -j2
mv host_sock host_mq host_fifo ..
cd ..
rm -r build
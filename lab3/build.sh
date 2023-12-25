#!/bin/bash

DIR_NAME="build"
EXE_NAME="lab3"

mkdir $DIR_NAME; cd $DIR_NAME
cmake ..; make
cd ..
cp $DIR_NAME/$EXE_NAME .
rm -r $DIR_NAME
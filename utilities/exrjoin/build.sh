#!/bin/bash

name="exrjoin"

cmd="g++ -c -o ${name}.o -Wall -O2 -DNDEBUG -I/usr/include/OpenEXR ${name}.cpp"
echo $cmd
$cmd

[ $? != "0" ] && exit 1

cmd="g++ -s -shared-libgcc -o ${name} ${name}.o -lIlmImf"
echo $cmd
$cmd

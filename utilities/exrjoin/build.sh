#!/bin/bash

name="exrjoin"

CPPFLAGS="-I/usr/include/OpenEXR"
LDFLAGS="-lIlmImf"

[ -f override.sh ] && source override.sh

cmd="g++ -c -o ${name}.o -Wall -O2 -DNDEBUG $CPPFLAGS ${name}.cpp"
echo $cmd
$cmd

[ $? != "0" ] && exit 1

cmd="g++ -s -o ${name} ${name}.o $LDFLAGS"
echo $cmd
$cmd

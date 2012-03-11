#!/bin/bash

prefix=$PWD/faac

cd faac-*

export LDFLAGS="$LDFLAGS -B/usr/lib/gold-ld/"

if [ ! -z "$1" ]; then
   ./configure --help
   exit
else
   ./configure --prefix=$prefix --enable-shared=
   ../patch_faac.py
   make && make install
fi

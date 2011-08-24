#!/bin/bash

ver=4.7.3
qtsrc=qt-everywhere-opensource-src-$ver
qtdir=$PWD/$ver

if [ ! -d $qtsrc ]; then
   echo "Qt sources folder '$qtsrc' does not exist."
   exit 1
fi

flags="-prefix $qtdir -v -opensource -release -static -qt-sql-psql -nomake examples -nomake demos -no-qt3support -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-gif -no-openssl"
if [ `uname` == "Darwin" ]; then
   echo "Darwin framework disabled."
   flags="$flags -no-framework -arch x86_64"
fi

echo "Flags: $flags"

cd $qtsrc

if [ "$1" == "-h" ]; then
   ./configure -h
else
   ./configure $flags "$@"
   make
   make install
fi

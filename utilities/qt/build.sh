#!/bin/bash

ver=4.8.4
qtsrc=qt-everywhere-opensource-src-$ver
[ -d $qtsrc ] || ver=4.8.5
qtsrc=qt-everywhere-opensource-src-$ver

for arg in $*; do
   [ "$arg" == "-h" ] && help=1
done

qtdir=$PWD/$ver

if [ ! -d $qtsrc ]; then
   echo "Qt sources folder '$qtsrc' does not exist."
   exit 1
fi

flags="-prefix $qtdir -v -opensource -release -static -silent"
flags="$flags -nomake examples -nomake demos -nomake designer"
flags="$flags -no-qt3support -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-gif -no-openssl"

if [ `uname` == "Darwin" ]; then
   echo "Darwin framework disabled."
   flags="$flags -no-framework -arch x86_64"
fi

cd $qtsrc

if [ ! -z "$help" ]; then
   ./configure -h
   echo "Flags: $flags"
else
   echo "Flags: $flags"
   ./configure $flags
   make
   make install
fi

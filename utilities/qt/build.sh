#!/bin/bash

ver=4.7.4
[ -z "$1" ] || ver=$1

for arg in $*; do
   [ "$arg" == "--nosql" ] && nosql=1
done

qtsrc=qt-everywhere-opensource-src-$ver
qtdir=$PWD/$ver

if [ ! -d $qtsrc ]; then
   echo "Qt sources folder '$qtsrc' does not exist."
   exit 1
fi

flags="-prefix $qtdir -v -opensource -release -static"
[ -z "$nosql" ] && flags="$flags -qt-sql-psql"
flags="$flags -nomake examples -nomake demos"
flags="$flags -no-qt3support -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-gif -no-openssl"

if [ `uname` == "Darwin" ]; then
   echo "Darwin framework disabled."
   flags="$flags -no-framework -arch x86_64"
fi

echo "Flags: $flags"

cd $qtsrc

if [ "$1" == "-h" ]; then
   ./configure -h
else
   ./configure $flags
   make
   make install
fi

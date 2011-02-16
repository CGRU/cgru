#!/bin/bash

ver=4.7.1
qtsrc=qt-everywhere-opensource-src-$ver
qtdir=$PWD/$ver

if [ ! -d $qtsrc ]; then
   echo "Qt sources folder '$qtsrc' does not exist."
   exit 1
fi

cd $qtsrc

if [ ! -z "$1" ]; then
   ./configure -h
else
   ./configure -prefix $qtdir -v -opensource -release -static -qt-sql-psql -nomake examples -nomake demos -no-qt3support -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-gif -no-openssl
   make
   make install
fi

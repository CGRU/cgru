#!/bin/bash

qt=4.7.1-static
qtsrc=qt-everywhere-opensource-src-$qt
qtdir=$PWD/qt-$qt

cd $qtsrc

if [ ! -z "$2" ]; then
   ./configure -h
else
   ./configure -prefix $qtdir -v -opensource -release -static -qt-sql-psql -nomake examples -nomake demos -no-qt3support -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-gif -no-openssl
   make
   make install
fi

#!/bin/bash

qt=4.7.0-static
qtsrc=qt-everywhere-opensource-src-$qt
qtdir=$PWD/qt-$qt

cd $qtsrc

./configure -prefix $qtdir -v -opensource -release -static -qt-sql-psql -nomake examples -nomake demos -no-qt3support -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-svg -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-gif -no-openssl

make
make install

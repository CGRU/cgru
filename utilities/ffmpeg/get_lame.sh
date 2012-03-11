#!/bin/bash

ver0="3.99"
ver1="3"
ver="$ver0.$ver1"
arch="lame-$ver.tar.gz"

[ -f "$arch" ] || wget http://sourceforge.net/projects/lame/files/lame/$ver0/$arch

tar xfvz "$arch"

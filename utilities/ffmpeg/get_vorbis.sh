#!/bin/bash

ver="1.3.0"
arch="libogg-$ver.tar.gz"
[ -f "$arch" ] || wget "http://downloads.xiph.org/releases/ogg/$arch"
tar xfvz "$arch"

ver="1.3.2"
arch="libvorbis-$ver.tar.bz2"
[ -f "$arch" ] || wget "http://downloads.xiph.org/releases/vorbis/$arch"
tar xfvj "$arch"

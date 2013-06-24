#!/bin/bash

ver="1.3.1"
arch="libogg-$ver.tar.xz"
[ -f "$arch" ] || wget "http://downloads.xiph.org/releases/ogg/$arch"
tar xfvJ "$arch"

ver="1.3.3"
arch="libvorbis-$ver.tar.xz"
[ -f "$arch" ] || wget "http://downloads.xiph.org/releases/vorbis/$arch"
tar xfvJ "$arch"

ver="1.1.1"
arch="libtheora-$ver.tar.xz"
[ -f "$arch" ] || wget "http://downloads.xiph.org/releases/theora/$arch"
tar xfvJ "$arch"

#!/bin/bash

ver="1.28"
arch="faac-$ver.tar.bz2"

[ -f "$arch" ] || wget "http://downloads.sourceforge.net/faac/$arch"

tar xfvj "$arch"

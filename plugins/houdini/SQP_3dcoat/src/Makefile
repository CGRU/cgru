SHELL=/bin/csh

HOUDINI_MAJOR_RELEASE=11
HOUDINI_MINOR_RELEASE=0
VERSION=${HOUDINI_MAJOR_RELEASE}.${HOUDINI_MINOR_RELEASE}
SRC_VER = 0.0.1
CFLAGS := $(CFLAGS) -DH9_1 -DSOP_MAJOR_VER=0 -DSOP_MINOR_VER=1
SHELL=/bin/csh

src = SOP_3dcoat.c SOP_3dcoat.h

all: SOP_3dcoat

SOP_3dcoat: ${src}
	hcustom -g -i . $@.c

ifeq ($(OS),Windows_NT)
	cp $@.dll ../dso/
else
	cp $@.so ../dso/
endif  


clean:
	rm *.o *.so
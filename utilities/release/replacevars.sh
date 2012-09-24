#!/bin/bash

input=$1
output=$2

if [ ! -f ${input} ]; then
   echo "File '$input' does not exist."
   echo "$0: No input file founded."
   exit 1
fi

sed \
-e "s/@VERSION@/${VERSION_NUMBER}/g" \
-e "s/@RELEASE@/${RELEASE_NUMBER}/g" \
-e "s/@ARCHITECTURE@/${ARCHITECTURE}/g" \
-e "s/@DEPENDS_AFANASY@/${DEPENDS_AFANASY}/g" \
-e "s/@DEPENDS_AFANASY_COMMA@/${DEPENDS_AFANASY_COMMA}/g" \
-e "s/@DEPENDS_QTGUI@/${DEPENDS_QTGUI}/g" \
-e "s/@DEPENDS_QTGUI_COMMA@/${DEPENDS_QTGUI_COMMA}/g" \
-e "s/@DEPENDS_CGRU@/${DEPENDS_CGRU}/g" \
-e "s/@DEPENDS_CGRU_COMMA@/${DEPENDS_CGRU_COMMA}/g" \
-e "s/@SIZE@/${SIZE}/g" \
< "${input}" > "${output}"

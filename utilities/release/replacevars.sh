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
-e "s/@DEPENDS_CGRU@/${DEPENDS_CGRU}/g" \
-e "s/@SIZE@/${SIZE}/g" \
< "${input}" > "${output}"

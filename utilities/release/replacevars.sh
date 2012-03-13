#/bin/bash

input=$1
output=$2

sed \
-e "s/@VERSION@/${VERSION_NUMBER}/g" \
-e "s/@ARCHITECTURE@/${ARCHITECTURE}/g" \
-e "s/@QT_VERSION@/${QT_VERSION}/g" \
-e "s/@PYTHON_VERSION@/${PYTHON_VERSION}/g" \
-e "s/@SIZE@/${SIZE}/g" \
< "${input}" > "${output}"

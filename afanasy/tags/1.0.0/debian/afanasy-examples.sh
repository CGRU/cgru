#!/bin/bash

# Initialize variables:
src=$1
pack=$2
cgru=$3
[ -z $cgru ] && exit

# Copying files:
mkdir -p $pack/$cgru/afanasy
cp -r $src/afanasy/examples $pack/$cgru/afanasy

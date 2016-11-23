#!/bin/bash

dest="/media/sf_cgru/utilities/release/archives/linux"

[ -d "${dest}" ] && rm -rf "${dest}"

mkdir "${dest}"

[ -d "${dest}" ] || exit 1

./export.sh $dest


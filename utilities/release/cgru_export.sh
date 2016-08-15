#!/bin/bash

dest="/media/sf_data/cgru-data/exports/cgru-linux"

[ -d "${dest}" ] && rm -rf "${dest}"

mkdir "${dest}"

[ -d "${dest}" ] || exit 1

./export.sh "$dest"

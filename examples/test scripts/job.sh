#!/usr/bin/bash

cur_folder=`pwd`

src_folder="`dirname "$0"`"

cd "${src_folder}"

source ./setup.sh

cd "${cur_folder}"

"${CGRU_PYTHONEXE}" "${src_folder}/job.py" "$@"

#!/bin/bash

source `dirname $0`/setup.sh
"$CGRU_PYTHONEXE" `dirname $0`/scandialog.py "$@"

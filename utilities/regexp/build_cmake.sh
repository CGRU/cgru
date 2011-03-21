#!/bin/bash

[ -f override.sh ] && source override.sh

cmake src && make

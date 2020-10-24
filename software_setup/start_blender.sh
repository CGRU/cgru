#!/bin/bash

source `dirname $0`/start__all.sh

blender --python-use-system-env "$@"

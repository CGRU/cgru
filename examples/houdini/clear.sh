#!/bin/bash

echo "Clearing Houdini examples in '$PWD'"

rm -fv *.hip*.hip
[ -d "backup" ] && rm -rvf "backup"
rm -fv render/*
rm -fv *.mov

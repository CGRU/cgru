#!/bin/bash

for scene in `ls scene.hip*`; do
   [ $scene == "scene.hip" ] && continue
   rm -fv $scene
done
[ -d "backup" ] && rm -rvf "backup"
rm -fv render/*

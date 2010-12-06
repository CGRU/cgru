#!/bin/bash

for scene in `ls scene.nk*`; do
   [ $scene == "scene.nk" ] && continue
   rm -fv $scene
done
rm -fv render/*
rm -fv *.mov

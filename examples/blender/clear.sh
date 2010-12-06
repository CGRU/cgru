#!/bin/bash

for scene in `ls scene.blend*`; do
   [ $scene == "scene.blend" ] && continue
   rm -fv $scene
done
rm -fv render/*

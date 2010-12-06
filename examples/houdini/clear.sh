#!/bin/bash

for scene in `ls scene.hip*`; do
   [ $scene == "scene.hip" ] && continue
   rm -fv $scene
done
rm -fv render/*

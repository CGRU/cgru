#!/bin/bash

scenes="scene.nk scene_stereo.nk"
for scene in $scenes; do
   for tmpscene in `ls ${scene}*`; do
      [ $tmpscene == $scene ] && continue
      rm -fv $tmpscene
   done
done
rm -fv render/*
rm -fv *.mov


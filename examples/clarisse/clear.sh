#!/bin/bash

echo "Clearing Clarisse examples in '$PWD'"

rm -fv *.autosave

rm -fv scene.af*.render

[ -d render ] && rm -rfv render

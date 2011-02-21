#!/bin/bash

cd temp/wwwroot

rsync -av --delete * timurhai,cgru@web.sourceforge.net:htdocs

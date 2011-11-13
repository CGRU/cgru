@echo off

set dest=%1
if exist %dest% rd /s /q %dest%
call minimize_python.cmd
cd ..\..
set python=%CD%\python

if [%1] == [] (
   echo Error: Destination not specified
   exit 1
)
if not exist %python% (
   echo Error: Python "%python%" does not exist
   exit 1
)

xcopy python %dest%\ /ys

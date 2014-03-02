@echo off

pushd ..\..
set python=%CD%\python

if not exist %python% (
   echo Error: Python "%python%" does not exist
   exit 1
)

cd %python%
call :dd Doc
call :dd include
call :dd libs
call :dd tcl
call :dd Tools
call :df qt.conf

cd Lib
call :dd test

cd site-packages
call :dd PySide-1.2.1-py3.3.egg-info
call :dd pysideuic

cd PySide
call :dd examples
call :df *.exe
call :dd docs
call :df libmysql*
call :df phonon*
call :df QAxContainer*
call :df QtCLucene*
call :df QtDeclarative*
call :df QtDesigner*
call :df QtHelp*
call :df QtMultimedia*
call :df QtOpenGL*
call :df QtScript*
call :df QtTest*
call :df QtWebKit*
call :df QtXmlPatterns*
call :dd translations

popd

goto :eof

:df
if exist %1 del /f /q %1
goto :eof

:dd
if exist %1 rd /s /q %1
goto :eof
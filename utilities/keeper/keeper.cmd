rem echo off

cd %0\..
set CGRU_KEEPER=%CD%
pushd ..\..
if exist setup.cmd call setup.cmd
popd

if [%1] == [] (
   start "Keeper" "%CGRU_PYTHONDIR%\pythonw.exe" keeper.py
) else (
   "%CGRU_PYTHONEXE%" keeper.py
)

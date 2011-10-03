rem echo off

cd %0\..
set CGRU_KEEPER=%CD%
echo %CGRU_PYTHONEXE%
pushd ..\..
if exist setup.cmd call setup.cmd
popd

if [%1] == [] (
   start "Keeper" "%CGRU_PYTHONDIR%\pythonw.exe" keeper.py
) else (
   "%CGRU_PYTHONEXE%" keeper.py
)

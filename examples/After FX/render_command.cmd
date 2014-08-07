pushd ..\..
call setup.cmd
popd

if not exist render mkdir render

aerender -project "%CD%\comp.aep" -comp "Comp 1" -s 1 -e 11

pause

pushd ..\..
call setup.cmd
popd

if not exist render mkdir render

aerender -project "%CD%\AE Project.aep" -comp "Comp 1" -s 1 -e 11
rem aerender -project "%CD%\AE Project.aep" -comp "Comp 1" -s 1 -e 1 -i 1 -mp -output "%CD%/render/Comp 1_[####].jpg"

pause

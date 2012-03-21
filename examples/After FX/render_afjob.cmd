pushd ..\..
call setup.cmd
popd

if not exist render mkdir render

%CGRU_PYTHONEXE% "%AF_ROOT%\python\afjob.py" "%CD%\AE Project.aep" 1 10 -fpt 2 -node "Comp 1" -output "%CD%\render\Comp 1_[####].jpg"

pause

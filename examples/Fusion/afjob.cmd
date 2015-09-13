pushd ..\..
call setup.cmd
popd

%CGRU_PYTHONEXE% "%AF_ROOT%\python\afjob.py" "%CD%\Composition1.comp" 1 10 -fpt 2

pause

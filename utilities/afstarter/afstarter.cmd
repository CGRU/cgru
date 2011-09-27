cd %0\..
pushd ..\..
call setup.cmd
popd

if [%1] == [] (
   start "AF Starter" pythonw afstarter.py
) else (
   python afstarter.py %*
)

cd %0\..
if not defined CGRU_LOCATION (
   pushd ..\..
   call setup.cmd
   popd
)

start "RegExp" %CGRU_PYTHONDIR%\pythonw.exe regexp.py

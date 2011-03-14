echo off
SET MXX_RU_CPP_TOOLSET=vc9

set output=..\..\bin_pyaf\%AF_PYAFVER%
set pyaf=tmp\%AF_PYAFVER%\pyaf.pyd

call %MSVCPATH%\vcvarsall.bat %1

ruby afanasy.mxw.rb --mxx-show-tmps

if not exist %output% mkdir %output%
copy %pyaf% %output%

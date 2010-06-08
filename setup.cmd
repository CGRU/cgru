rem Set CGRU root:
SET CGRU_LOCATION=%CD%

rem Add CGRU bin to path:
SET PATH=%CGRU_LOCATION%\bin;%PATH%

rem Python module path:
SET CGRU_PYTHON=%CGRU_LOCATION%\lib\python
if defined PYTHONPATH (
   SET PYTHONPATH=%CGRU_PYTHON%;%PYTHONPATH%
) else (
   SET PYTHONPATH=%CGRU_PYTHON%
)

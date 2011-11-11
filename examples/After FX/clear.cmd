pushd "%0\.."

echo "Clearing AfterFX examples in %CD%"

del "AE Project.aep*.aep"
rd /s /q "AE Project.aep Logs"
cd render
del /f /q *

popd

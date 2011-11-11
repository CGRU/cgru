pushd "%0\.."

echo "Clearing Max examples in %CD%"

del scene.max*.max

cd render
del /f /q *

popd

pushd "%0\.."

echo "Clearing Maya examples in %CD%"

del scene.mb*.mb

cd render
del /f /q *

popd

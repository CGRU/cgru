pushd "%0\.."

echo "Clearing Blender examples in %CD%"

del scene.blend*.blend

cd render
del /f /q *

popd

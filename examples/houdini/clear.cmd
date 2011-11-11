pushd "%0\.."

echo "Clearing Houdini examples in %CD%"

del scene.hip*.hip
del /f /q *.mov

cd render
del /f /q *

popd

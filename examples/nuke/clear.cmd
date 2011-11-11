pushd "%0\.."

echo "Clearing Nuke examples in %CD%"

del scene.nk*.nk
del /f /q *.mov

cd render
del /f /q *

popd

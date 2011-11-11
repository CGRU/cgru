pushd "%0\.."

echo "Clearing XSI examples in %CD%"

cd project
rd /s /q Backup

cd Scenes
del scene.scn*.scn
cd ..

cd Render_Pictures
del /f /q *
rd /s /q 20 80 140 200

popd

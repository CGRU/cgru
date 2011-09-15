pushd ..\..
call setup.cmd
popd

3dsmaxcmd "%CD%\scene.max" -start:1 -end:10 -v:5 -showRFW:0 -o:render/render_img.0000.jpg

pause

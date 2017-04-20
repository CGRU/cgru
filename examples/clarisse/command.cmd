PUSHD ..\..
call setup.cmd
POPD

if not exist render mkdir render

clarisse_node "%CD%\archive.render" -image "scene/image" -start_frame "1" -end_frame "2" -output "%CD%\render\img.####" -format "jpg" -log_width "0"

pause

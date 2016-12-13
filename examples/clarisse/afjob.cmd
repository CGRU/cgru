PUSHD ..\..
call setup.cmd
POPD

if not exist render mkdir render

rem clarisse_node "$PWD/archive.render" -image "scene/image" -start_frame "1" -end_frame "2" -output "$PWD/render/img.####" -format "jpg" -log_width "0"

%CGRU_PYTHONEXE% "%CGRU_LOCATION%\afanasy\python\afjob.py" "%CD%\archive.render" "1" "10" -fpt "1" -node "scene/image" -output "%CD%\render\img.####"

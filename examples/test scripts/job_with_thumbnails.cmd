PUSHD ..\..
call setup.cmd
POPD

if not exist render mkdir render

%CGRU_PYTHONEXE% job.py --filesout "render\img.@####@.jpg" %*

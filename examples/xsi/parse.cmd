PUSHD ..
call setup.cmd
POPD

python "%AF_ROOT%/python/parse.py" xsi 3 xsibatch.cmd -render "%CD%\project\Scenes\scene.scn" -frames 1,5,2 -pass Default_Pass -verbose on

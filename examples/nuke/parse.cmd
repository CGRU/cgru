PUSHD ..
call setup.cmd
POPD

python "%AF_ROOT%/python/parse.py" nuke 3 nuke.cmd -X back -x scene.nk 1,3

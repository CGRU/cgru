PUSHD ..
call setup_houdini.cmd
POPD

python render.py %*

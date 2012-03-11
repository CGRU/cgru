PUSHD ..
call setup.cmd
POPD

python "%AF_ROOT%/python/parse.py" hbatch_mantra 3 hrender_af.cmd -s 1 -e 3 -b 1 scene.hip /out/image

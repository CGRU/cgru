PUSHD ..
call setup_nuke.cmd
POPD

%NUKE_EXEC% %*

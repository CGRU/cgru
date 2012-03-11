@echo off

IF NOT DEFINED AF_ROOT (
	PUSHD afanasy
   if exist trunk cd trunk
	if exist setup.cmd call setup.cmd
	POPD
)

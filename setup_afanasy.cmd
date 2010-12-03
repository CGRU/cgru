@echo off

IF NOT DEFINED AF_ROOT (
	PUSHD afanasy
   if exist trunk cd trunk
	call setup.cmd
	POPD
)

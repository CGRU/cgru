@echo off

IF NOT DEFINED AF_ROOT (
	PUSHD afanasy
	call setup.cmd
	POPD
)

pushd ..\..
call setup.cmd
popd

if not defined AF_SRV_USER (
	if defined CG_USER (
		set "AF_SRV_USER=.\%CG_USER%"
	)
)

if not defined AF_SRV_PASS (
	if defined CG_PASS (
		set "AF_SRV_PASS=%CG_PASS%"
	)
)

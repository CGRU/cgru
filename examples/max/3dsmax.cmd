call setup_max.cmd

IF [%1]==[] (
set TOSTART=start
) ELSE (
set TOSTART=
)

%TOSTART% 3dsmax.exe %*

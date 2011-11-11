@echo off

cd "%0\.."

For /F "Tokens=*" %%I in ('dir /b /ad') Do (
   pushd "%%I"
   if exist clear.cmd call clear.cmd
   popd
)

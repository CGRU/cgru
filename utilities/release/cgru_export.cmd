set "DEST=T:\cgru\utilities\release\archives\windows"

if exist "%DEST%" rd /s /q "%DEST%"

call export.cmd "%DEST%"
call export_python.cmd "%DEST%/python"

pause

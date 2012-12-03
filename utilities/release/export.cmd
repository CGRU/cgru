set dest=%1

if exist override.cmd call override.cmd

cd ..\..

set cgru=%CD%

cd examples
call clear.cmd

cd "%cgru%"

if not exist %dest% mkdir %dest%

set afanasy=trunk

xcopy . %dest%\ /y

xcopy bin %dest%\bin\ /ys
xcopy lib %dest%\lib\ /ys
xcopy icons %dest%\icons\ /ys
xcopy examples %dest%\examples\ /ys
xcopy software_setup %dest%\software_setup\ /ys
xcopy start %dest%\start\ /ys

xcopy afanasy %dest%\afanasy\ /y
xcopy afanasy\bin %dest%\afanasy\bin\ /y
del %dest%\afanasy\bin\afmonitor.exe
xcopy afanasy\icons %dest%\afanasy\icons\ /ys
xcopy afanasy\plugins %dest%\afanasy\plugins\ /ys
xcopy afanasy\python %dest%\afanasy\python\ /ys

xcopy utilities\ %dest%\utilities\ /y
xcopy utilities\keeper %dest%\utilities\keeper\ /ys
xcopy utilities\moviemaker %dest%\utilities\moviemaker\ /ys
xcopy utilities\afstarter %dest%\utilities\afstarter\ /ys
xcopy utilities\regexp %dest%\utilities\regexp\ /ys
xcopy utilities\*.py %dest%\utilities\ /y
xcopy utilities\*.cmd %dest%\utilities\ /y

xcopy plugins %dest%\plugins\ /y
xcopy plugins\nuke %dest%\plugins\nuke\ /ys
xcopy plugins\houdini %dest%\plugins\houdini\ /ys
xcopy plugins\xsi %dest%\plugins\xsi\ /ys
xcopy plugins\blender %dest%\plugins\blender\ /ys
xcopy plugins\max %dest%\plugins\max\ /ys
xcopy plugins\c4d %dest%\plugins\c4d\ /ys

xcopy plugins\maya %dest%\plugins\maya\ /y
xcopy plugins\maya\icons %dest%\plugins\maya\icons\ /ys
xcopy plugins\maya\mel %dest%\plugins\maya\mel\ /ys
xcopy plugins\maya\mll %dest%\plugins\maya\mll\ /ys

rem Get and store version and revision:
For /F "Tokens=*" %%I in ('type version.txt') Do Set CGRU_VERSION=%%I
cd utilities
call getrevision.cmd "%cgru%"
rem echo %CGRU_VERSION% rev%CGRU_REVISION% > %dest%\version.txt
echo %CGRU_REVISION% > %dest%\revision.txt

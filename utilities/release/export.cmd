set dest=%1

if exist override.cmd call override.cmd

pushd ..\..

set cgru=%CD%

cd examples
call clear.cmd

cd "%cgru%"

if not exist %dest% mkdir %dest%

xcopy . %dest%\ /y

xcopy bin %dest%\bin\ /ys
xcopy dll %dest%\dll\ /ys
xcopy examples %dest%\examples\ /ys
xcopy icons %dest%\icons\ /ys
xcopy lib %dest%\lib\ /ys
xcopy rules %dest%\rules\ /ys
xcopy software_setup %dest%\software_setup\ /ys
xcopy start %dest%\start\ /ys

xcopy afanasy %dest%\afanasy\ /y
xcopy afanasy\bin %dest%\afanasy\bin\ /y
xcopy afanasy\bin\imageformats %dest%\afanasy\bin\imageformats\ /y
xcopy afanasy\bin\platforms %dest%\afanasy\bin\platforms\ /y
xcopy afanasy\bin\qml %dest%\afanasy\bin\qml\ /y
xcopy afanasy\browser %dest%\afanasy\browser\ /y
xcopy afanasy\icons %dest%\afanasy\icons\ /ys
xcopy afanasy\python %dest%\afanasy\python\ /ys
xcopy afanasy\statistics %dest%\afanasy\statistics\ /ys

xcopy utilities %dest%\utilities\ /y
xcopy utilities\keeper %dest%\utilities\keeper\ /ys
xcopy utilities\moviemaker %dest%\utilities\moviemaker\ /ys
xcopy utilities\afstarter %dest%\utilities\afstarter\ /ys
xcopy utilities\regexp %dest%\utilities\regexp\ /ys

xcopy plugins %dest%\plugins\ /y
xcopy plugins\afterfx %dest%\plugins\afterfx\ /ys
xcopy plugins\clarisse %dest%\plugins\clarisse\ /ys
xcopy plugins\c4d %dest%\plugins\c4d\ /ys
xcopy plugins\blender %dest%\plugins\blender\ /ys
xcopy plugins\fusion %dest%\plugins\fusion\ /ys
xcopy plugins\houdini %dest%\plugins\houdini\ /ys
xcopy plugins\max %dest%\plugins\max\ /ys
xcopy plugins\natron %dest%\plugins\natron\ /ys
xcopy plugins\nuke %dest%\plugins\nuke\ /ys
xcopy plugins\xsi %dest%\plugins\xsi\ /ys

xcopy plugins\maya %dest%\plugins\maya\ /y
xcopy plugins\maya\afanasy %dest%\plugins\maya\afanasy\ /ys
xcopy plugins\maya\icons %dest%\plugins\maya\icons\ /ys
xcopy plugins\maya\mel %dest%\plugins\maya\mel\ /ys
xcopy plugins\maya\mll %dest%\plugins\maya\mll\ /ys

rem Get and store version and revision:
For /F "Tokens=*" %%I in ('type version.txt') Do Set CGRU_VERSION=%%I
cd utilities
call getrevision.cmd "%cgru%"
rem echo %CGRU_VERSION% rev%CGRU_REVISION% > %dest%\version.txt
echo %CGRU_REVISION% > %dest%\revision.txt

popd

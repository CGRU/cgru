set dest=%1

if exist override.cmd call override.cmd

PUSHD ..\..

if exist %dest% mkdir %dest%

set afanasy=trunk

xcopy . %dest%\ /y

xcopy bin %dest%\bin\ /ys
xcopy lib %dest%\lib\ /ys

xcopy doc %dest%\doc\ /y
xcopy doc\icons %dest%\doc\icons\ /ys
xcopy doc\images %dest%\doc\images\ /ys

xcopy afanasy\%afanasy% %dest%\afanasy\ /y
xcopy afanasy\%afanasy%\bin %dest%\afanasy\bin\ /y
xcopy afanasy\%afanasy%\bin_pyaf %dest%\afanasy\bin_pyaf\ /ys
xcopy afanasy\%afanasy%\icons %dest%\afanasy\icons\ /ys
xcopy afanasy\%afanasy%\plugins %dest%\afanasy\plugins\ /ys
xcopy afanasy\%afanasy%\python %dest%\afanasy\python\ /ys
xcopy afanasy\%afanasy%\doc %dest%\afanasy\doc\ /ys
xcopy afanasy\%afanasy%\launch %dest%\afanasy\launch\ /y

xcopy utilities\ %dest%\utilities\ /y
xcopy utilities\doc %dest%\utilities\doc\ /ys
xcopy utilities\keeper %dest%\utilities\keeper\ /ys
xcopy utilities\moviemaker %dest%\utilities\moviemaker\ /ys

if not exist %dest%\utilities\regexp mkdir %dest%\utilities\regexp
xcopy utilities\regexp\bin %dest%\utilities\regexp\bin\ /y
xcopy utilities\regexp\doc %dest%\utilities\regexp\doc\ /ys
xcopy utilities\regexp\icons %dest%\utilities\regexp\icons\ /ys

xcopy plugins %dest%\plugins\ /y
xcopy plugins\nuke %dest%\plugins\nuke\ /ys
xcopy plugins\houdini %dest%\plugins\houdini\ /ys
xcopy plugins\xsi %dest%\plugins\xsi\ /ys
xcopy plugins\blender %dest%\plugins\blender\ /ys
xcopy plugins\max %dest%\plugins\max\ /ys

xcopy plugins\maya %dest%\plugins\maya\ /y
xcopy plugins\maya\doc %dest%\plugins\maya\doc\ /ys
xcopy plugins\maya\icons %dest%\plugins\maya\icons\ /ys
xcopy plugins\maya\mel %dest%\plugins\maya\mel\ /ys
xcopy plugins\maya\mll %dest%\plugins\maya\mll\ /ys

rem Get and store version and revision:
For /F "Tokens=*" %%I in ('type version.txt') Do Set CGRU_VERSION=%%I
cd utilities
call getrevision.cmd ..
echo CGRU_REVISION %CGRU_REVISION%
echo %CGRU_VERSION% rev%CGRU_REVISION% > %dest%\version.txt

POPD

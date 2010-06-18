set dest=%1

PUSHD ..\..

if exist %dest% mkdir %dest%

set src=\\soft\config\cgru\win
set afanasy=trunk

xcopy . %dest%\ /y

xcopy bin %dest%\bin\ /ys
xcopy lib %dest%\lib\ /ys

xcopy doc %dest%\doc\ /y
xcopy doc\icons %dest%\doc\icons\ /ys
xcopy doc\images %dest%\doc\images\ /ys

xcopy afanasy\%afanasy% %dest%\afanasy\ /y
xcopy afanasy\%afanasy%\bin %dest%\afanasy\bin\ /y
xcopy afanasy\%afanasy%\icons %dest%\afanasy\icons\ /ys
xcopy afanasy\%afanasy%\plugins %dest%\afanasy\plugins\ /ys
xcopy afanasy\%afanasy%\python %dest%\afanasy\python\ /ys

xcopy utilities\ %dest%\utilities\ /y
xcopy utilities\doc %dest%\utilities\doc\ /ys
xcopy utilities\moviemaker %dest%\utilities\moviemaker\ /ys

xcopy plugins %dest%\plugins\ /y
xcopy plugins\nuke %dest%\plugins\nuke\ /ys
xcopy plugins\houdini %dest%\plugins\houdini\ /ys
xcopy plugins\xsi %dest%\plugins\xsi\ /ys
xcopy plugins\blender %dest%\plugins\blender\ /ys

xcopy plugins\maya %dest%\plugins\maya\ /y
xcopy plugins\maya\doc %dest%\plugins\maya\doc\ /ys
xcopy plugins\maya\icons %dest%\plugins\maya\icons\ /ys
xcopy plugins\maya\mel %dest%\plugins\maya\mel\ /ys
xcopy plugins\maya\mll %dest%\plugins\maya\mll\ /ys

POPD

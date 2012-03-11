PUSHD ..\..
call setup.cmd
POPD

set XSI_CGRU_PATH=%CGRU_LOCATION%\plugins\xsi

rem xsibatch -render "%CD%\project\Scenes\scene.scn" -frames 1,5,2 -pass Default_Pass -verbose on

rem xsibatch -script c:\cg\tools\cgru\plugins\xsi\afrender.py -lang Python -main afRender -args -scene c:\cg\tools\cgru\afanasy\trunk\examples\xsi\project\Scenes\scene.scn -start 1 -end 1 -step 1 -simulate 1 -renderPass "Default_Pass" -attr torus.polymsh.geom.enduangle -value 220

python "%AF_ROOT%/python/afjob.py" "%CD%\project\Scenes\scene.scn" 1 2 -fpt 1 -take Default_Pass -name XSI_Job -varirender torus.polymsh.geom.enduangle 20 40 4

pause

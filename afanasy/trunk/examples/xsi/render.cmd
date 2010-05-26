call setup_xsi.cmd

rem xsibatch -render "%CD%\project\Scenes\scene.scn" -frames 1,5,2 -pass Default_Pass -verbose on

python "%AF_ROOT%/python/afjob.py" "%CD%\project\Scenes\scene.scn" 1 2 -fpr 1 -take Default_Pass -name XSI_Job

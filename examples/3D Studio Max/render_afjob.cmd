call setup_max.cmd

python %AF_ROOT%\python\afjob.py "%CD%\scene.max" 1 10 -fpt 2 -output "render\afjob_img.0000.jpg"

pause

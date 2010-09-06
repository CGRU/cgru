call setup_max.cmd

python %AF_ROOT%\python\afjob.py scene.max 1 10 -fpt 2 -image render\img.0000.jpg -tempscene -deletescene

pause

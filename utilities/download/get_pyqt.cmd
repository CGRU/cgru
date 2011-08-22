set filename=PyQt-Py2.7-x64-gpl-4.8.5-1.exe
set location=http://www.riverbankcomputing.co.uk/static/Downloads/PyQt4
if exist %filename% (
   echo File %filename% already exists. Skipping download.
) else (
   wget %location%/%filename%
)

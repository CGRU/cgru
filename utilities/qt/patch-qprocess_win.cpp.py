import os, sys

File = 'src\corelib\io\qprocess_win.cpp'
print('Trying to patch "%s":' % File)
FileBackup = File + '.cgru.backup'
LinesOrig = 'success = CreateProcess(0, (wchar_t*)args.utf16(),\n\
                            0, 0, TRUE, dwCreationFlags,\n\
                            environment.isEmpty() ? 0 : envlist.data(),\n\
                            workingDirectory.isEmpty() ? 0 : (wchar_t*)QDir::toNativeSeparators(workingDirectory).utf16(),\n\
                            &startupInfo, pid);'
LinesPatch = '//PATCH BY CGRU BEGIN\n\
success = CreateProcess(0, (wchar_t*)args.utf16(),\n\
0, 0, TRUE, dwCreationFlags | CREATE_SUSPENDED,\n\
environment.isEmpty() ? 0 : envlist.data(),\n\
workingDirectory.isEmpty() ? 0 : (wchar_t*)QDir::toNativeSeparators(workingDirectory).utf16(),\n\
&startupInfo, pid);\n\
//PATCH BY CGRU END\n'

def errorExit( msg = ''):
   print('Error: ' + msg)
   print('Usage: ' + sys.argv[0] + ' QtDirectory')
   sys.exit(1)
                     
if len( sys.argv) < 2: errorExit('Qt folder not specified.')

QtDir = sys.argv[1]

File = os.path.join( QtDir, File)
if not os.path.isfile( File): errorExit('File "%s" not founded.' % File)
FileBackup = os.path.join( QtDir, FileBackup)

# Search data:
FileObj = open( File, 'r')
Data = FileObj.read()
FileObj.close()
if Data.find( LinesPatch) != -1:
   print('Already patched.')
   sys.exit(0)
if Data.find( LinesOrig) == -1:
   print('Not Founded!')
   sys.exit(1)

# Create backup:
FileObj = open( FileBackup, 'w')
FileObj.write( Data)
FileObj.close()

# Create patched file:
Data = Data.replace( LinesOrig, LinesPatch)
FileObj = open( File, 'w')
FileObj.write( Data)
FileObj.close()

print('Patched.')

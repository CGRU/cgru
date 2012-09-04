QtCore = None
QtGui = 'QtGui'
QtNetwork = 'QtNetwork'
List = ['QtCore', 'QtGui', 'QtNetwork']
PythonQtType = None

try:
	PythonQt = __import__('PySide', globals(), locals(), List, -1)
	PythonQtType = 'PySide'
except:
	PythonQt = __import__('PyQt4', globals(), locals(), List, -1)
	PythonQtType = 'PyQt4'

QtCore = PythonQt.QtCore
QtGui = PythonQt.QtGui
QtNetwork = PythonQt.QtNetwork

print( PythonQtType + ' module imported.')
if PythonQtType == 'PyQt4':
	print('You can install PySide if interested in LGPL license.')

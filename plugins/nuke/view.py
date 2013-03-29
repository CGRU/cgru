import optparse, os, re, sys

# Parse arguments:
Parser = optparse.OptionParser( usage="usage: %prog [options] (like nuke --help)", version="%prog 1.0")
Parser.add_option('-o', '--open', dest='open', action='store_true', default=False, help='Open Nuke and create read nodes.')
Parser.add_option('-c', '--copy', dest='copy', action='store_true', default=False, help='Copy readers for paste in Nuke.')
(Options, Args) = Parser.parse_args()

if len( Args ) < 1: Parser.error('Input is not provided.')

Input = Args[-1]

print( Input)

InDir = Input

Prefixes = []
Suffixes = []
Paddings = []
Starts = []
Ends = []

if os.path.isdir( InDir):
	allitems = os.listdir( InDir)
	allitems.sort()
	for item in allitems:
		digits = re.findall('\d{1,}', item)
		if len(digits) == 0: continue
		digits = digits[-1]
		padding = len( digits)
		prefix = item[:item.rfind( digits)]
		suffix = item[len(prefix)+padding:]
		padding = len(item)-len(prefix)-len(suffix)
#		print( item + ': ' + prefix + ' ' + suffix + ' ' + str(padding))
		frame = int( item[len(prefix):len(prefix)+padding])
		if len(Prefixes) == 0 or Prefixes[-1] != prefix or Suffixes[-1] != suffix or Paddings[-1] != padding:
#			print( len(prefix), padding-1, item[len(prefix):len(prefix)+padding])
			Starts.append( frame)
			Ends.append( frame)
			Prefixes.append( prefix)
			Suffixes.append( suffix)
			Paddings.append( padding)
#			print( item)
		elif( len(Prefixes)):
			Ends[-1] = frame

InDir = InDir.replace('\\','/')

if Options.open:
	nuke = __import__('nuke', globals(), locals(), [])
if Options.copy:
	CopyText = 'set cut_paste_input [stack 0]\n'
	cgrupyqt = __import__('cgrupyqt', globals(), locals(), [])
	qapp = cgrupyqt.QtGui.QApplication([])

for i in range( len(Prefixes)):
	print('%s[%d-%d/%d]%s' % (Prefixes[i], Starts[i], Ends[i], Paddings[i], Suffixes[i]))
	filename = Prefixes[i] + '#'*Paddings[i] + Suffixes[i]
	filename = InDir + '/' + filename
	filename = filename.replace('//','/')
	print( filename)
	if Options.open:
		node = nuke.createNode('Read')
		node.knob('file').setValue( filename)
		node.knob('first').setValue( Starts[i])
		node.knob('last').setValue( Ends[i])
	if Options.copy:
		CopyText += 'Read {\n'
		CopyText += ' inputs 0\n'
		CopyText += ' file "%s"\n' % filename
		CopyText += ' first %d\n' % Starts[i]
		CopyText += ' last %d\n' % Ends[i]
		CopyText += ' selected true\n'
		CopyText += '}\n'


if Options.copy:
	print('Copying:')
	print( CopyText)
	qapp.clipboard().setText( CopyText)
	cgrupyqt.QtCore.QTimer.singleShot( 10000, qapp.quit)
#	qapp.processEvents()
#	qapp.sendPostedEvents()
#	clipboard = cgrupyqt.QtGui.QApplication.clipboard()
#	event = cgrupyqt.QtCore.QEvent( cgrupyqt.QtCore.QEvent.Clipboard)
#	cgrupyqt.QtGui.QApplication.sendEvent( clipboard, event)
	qapp.exec_()


import os, re, sys

Input = sys.argv[-1]

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

for i in range( len(Prefixes)):
	print('%s[%d-%d/%d]%s' % (Prefixes[i], Starts[i], Ends[i], Paddings[i], Suffixes[i]))
	filename = Prefixes[i] + '#'*Paddings[i] + Suffixes[i]
	filename = InDir + '/' + filename
	filename = filename.replace('//','/')
	print( filename)
	nuke = __import__('nuke', globals(), locals(), [])
	node = nuke.createNode('Read')
	node.knob('file').setValue( filename)
	node.knob('first').setValue( Starts[i])
	node.knob('last').setValue( Ends[i])


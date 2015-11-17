# Create prefs file:

import os

path = os.getenv('FUSION_CGRU_PATH')
path = path.replace('\\','\\\\')

prefs = os.getenv('FUSION_CGRU_PREFS')

fhandle = open( prefs,'w')
fhandle.write('{Global={Paths={Map={["Scripts:"]="%s\\\\;Fusion:Scripts",},},},}' % path)

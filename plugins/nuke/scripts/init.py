print 'Starting CGRU Nuke add-ons...'

import cgru
import nuke

# Delete current directory from paths:
nuke.addKnobChanged( cgru.delcurdir.call, nodeClass='Read')
nuke.addKnobChanged( cgru.delcurdir.call, nodeClass='Write')

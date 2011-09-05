import af
import cgruconfig

def setnimby( text = '(keeper)'):
   cmd = af.Cmd()
   cmd.setNimby( cgruconfig.VARS['hostname'] + '.*', text)
   cmd._sendRequest()

def setNIMBY( text = '(keeper)'):
   cmd = af.Cmd()
   cmd.setNIMBY( cgruconfig.VARS['hostname'] + '.*', text)
   cmd._sendRequest()

def setFree( text = '(keeper)'):
   cmd = af.Cmd()
   cmd.setFree( cgruconfig.VARS['hostname'] + '.*', text)
   cmd._sendRequest()

def ejectTasks( text = '(keeper)'):
   cmd = af.Cmd()
   cmd.ejectTasks( cgruconfig.VARS['hostname'] + '.*', text)
   cmd._sendRequest()

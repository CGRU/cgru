import af
import cgruconfig

def setnimby():
   cmd = af.Cmd()
   cmd.setNimby( cgruconfig.VARS['hostname'] + '.*')
   cmd._sendRequest()

def setNIMBY():
   cmd = af.Cmd()
   cmd.setNIMBY( cgruconfig.VARS['hostname'] + '.*')
   cmd._sendRequest()

def setFree():
   cmd = af.Cmd()
   cmd.setFree( cgruconfig.VARS['hostname'] + '.*')
   cmd._sendRequest()

def ejectTasks():
   cmd = af.Cmd()
   cmd.ejectTasks( cgruconfig.VARS['hostname'] + '.*')
   cmd._sendRequest()

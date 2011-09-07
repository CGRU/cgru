import cgruconfig
import nimby

from PyQt4 import QtCore

nimby_set = False
free_set = False

def refreshNimby():

   global nimby_set
   global free_set

   # Get today:
   datetime = QtCore.QDateTime.currentDateTime()
   time = datetime.toString('hh:mm')
   day = str(datetime.toString('ddd')).lower()
   var = 'nimby_' + day
   if var not in cgruconfig.VARS: return
   values = cgruconfig.VARS[var].split(' ')
   time_begin = values[0]
   time_end = values[1]

   # Check yesterday:
   yesterday = str(datetime.addDays(-1).toString('ddd')).lower()
   var_y = 'nimby_' + yesterday
   if var_y in cgruconfig.VARS and time < time_begin:
      time_end_y = cgruconfig.VARS[var_y].split(' ')[1]
      if time_end_y < time_begin and not free_set:
         nimby.setFree('(keeper nimby schedule)')
         free_set = True
         nimby_set = False

#   print('nimby.nimby():','(',yesterday,time_end_y,')',day,time,cgruconfig.VARS[var])

   if time_begin == time_end: return

   allow = False
   eject = False
   if len(values) > 2:
      if 'a' in values[2]: allow = True
      if 'e' in values[2]: eject = True
   if time > time_end and not free_set:
      nimby.setFree('(keeper nimby schedule)')
      free_set = True
      nimby_set = False
   elif time > time_begin and not nimby_set:
      if allow: nimby.setnimby('(keeper nimby schedule)')
      else: nimby.setNIMBY('(keeper nimby schedule)')
      if eject: nimby.ejectTasks('(keeper nimby schedule)')
      nimby_set = True
      free_set = False

def refresh():
   cgruconfig.Config()
   refreshNimby()

import af
import cgruconfig

from datetime import datetime,timedelta

nimby_set = False
free_set = False

def setnimby(   text = '(keeper)'): cmd = af.Cmd().renderSetNimby(   text)
def setNIMBY(   text = '(keeper)'): cmd = af.Cmd().renderSetNIMBY(   text)
def setFree(    text = '(keeper)'): cmd = af.Cmd().renderSetFree(    text)
def ejectTasks( text = '(keeper)'): cmd = af.Cmd().renderEjectTasks( text)

def refresh():

   global nimby_set
   global free_set

   # Get today:
   daytime = datetime.now()
   time = daytime.strftime('%H:%M')
   day = daytime.strftime('%a').lower()
   var = 'nimby_' + day
   if var not in cgruconfig.VARS: return
   values = cgruconfig.VARS[var].split(' ')
   time_begin = values[0]
   time_end = values[1]

   # Check yesterday:
   yesterday = (daytime-timedelta(days=1)).strftime('%a').lower()
   var_y = 'nimby_' + yesterday
   if var_y in cgruconfig.VARS and time < time_begin:
      time_end_y = cgruconfig.VARS[var_y].split(' ')[1]
      if time_end_y < time_begin and not free_set:
         setFree('(keeper nimby yesterday schedule)')
         free_set = True
         nimby_set = False

   if time_begin == time_end: return

   allow = False
   eject = False
   if len(values) > 2:
      if 'a' in values[2]: allow = True
      if 'e' in values[2]: eject = True
   if time > time_end:
      if not free_set:
         setFree('(keeper nimby schedule)')
         free_set = True
         nimby_set = False
   elif time > time_begin:
      if not nimby_set:
         if allow: setnimby('(keeper nimby schedule)')
         else: setNIMBY('(keeper nimby schedule)')
         if eject: ejectTasks('(keeper nimby schedule)')
         nimby_set = True
         free_set = False

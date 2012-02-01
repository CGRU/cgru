import af
import cgruconfig

from datetime import datetime,timedelta

nimby_set = False
free_set = False

def setnimby(        text = '(keeper)'): cmd = af.Cmd().renderSetNimby(        text )
def setNIMBY(        text = '(keeper)'): cmd = af.Cmd().renderSetNIMBY(        text )
def setFree(         text = '(keeper)'): cmd = af.Cmd().renderSetFree(         text )
def ejectTasks(      text = '(keeper)'): cmd = af.Cmd().renderEjectTasks(      text )
def ejectNotMyTasks( text = '(keeper)'): cmd = af.Cmd().renderEjectNotMyTasks( text )

def refresh( reset = False):
   global nimby_set
   global free_set

   if reset:
      nimby_set = False
      free_set = False

   toset_nimby = False
   toset_free  = False
   toallow_tasks = False
   toeject_tasks = False

   # Get current time and day:
   daytime = datetime.now()
   cur_time = daytime.strftime('%H:%M')

   # Check yesterday free:
   yesterday = (daytime-timedelta(days=1)).strftime('%a').lower()
   var = 'nimby_' + yesterday
   if var in cgruconfig.VARS:
      values = cgruconfig.VARS[var].split(' ')
      time_begin = values[0]
      time_end = values[1]
      if time_end < time_begin and 'Enable' in values:
         if cur_time > time_end:
            toset_free = True
         else:
            toset_nimby = True
            if 'Eject' in values: toeject_tasks = True
            if 'nimby' in values: toallow_tasks = True
            if 'NIMBY' in values: toallow_tasks = False

   # Check today:
   var = 'nimby_' + daytime.strftime('%a').lower()
   if var in cgruconfig.VARS:
      values = cgruconfig.VARS[var].split(' ')
      time_begin = values[0]
      time_end = values[1]
      if time_begin != time_end and 'Enable' in values:
         # Check free if it not tommorow:
         if time_end > time_begin and cur_time > time_end:
            toset_free = True
         # Check Nimby:
         elif cur_time > time_begin:
            toset_nimby = True
            if 'Eject' in values: toeject_tasks = True
            if 'nimby' in values: toallow_tasks = True
            if 'NIMBY' in values: toallow_tasks = False

   # Set state:
   if toset_nimby:
      if not nimby_set:
         if toallow_tasks:
            setnimby('(keeper nimby schedule)')
            if toeject_tasks: ejectNotMyTasks('(keeper nimby schedule)')
         else:
            setNIMBY('(keeper nimby schedule)')
            if toeject_tasks: ejectTasks('(keeper nimby schedule)')
         nimby_set = True
         free_set = False
   elif toset_free:
      if not free_set:
         setFree('(keeper nimby schedule)')
         free_set = True
         nimby_set = False

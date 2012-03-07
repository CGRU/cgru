#!/bin/bash
#
# chkconfig: 2345 90 10
# description: Afanasy @APPLICATION@ daemon
#
### BEGIN INIT INFO
# Provides: @APPLICATION@
# Required-Start:  @DEPENDS@
# Required-Stop:   @DEPENDS@
# Should-Start:    $syslog
# Should-Stop:     $syslog
# Default-Start:   2 3 4 5
# Default-Stop:    0 1 6
# Short-Description: Afanasy @APPLICATION@ daemon
### END INIT INFO

# Set Afanasy application:
afapp=@APPLICATION@

# Set Afanasy location:
afroot=@AFROOT@

# Afanasy "nonroot" user:
nonrootuser=render
if [ -f "${afroot}/nonrootuser" ]; then
   nonrootuser=`cat "${afroot}/nonrootuser"`
else
   echo "${nonrootuser}" > "${afroot}/nonrootuser"
fi
id "${nonrootuser}" || nonrootuser=""

# Temp directory:
tmpdir=/var/tmp/afanasy
if [ -d "$tmpdir" ]; then
   # Delete old temporary folders
   find $tmpdir -type d -mtime +99 -exec rm -rvf {} \;
   # Delete old temporary files
   find $tmpdir -mtime +99 -exec rm -vf {} \;
else
   mkdir $tmpdir
   if [ ! -d "$tmpdir" ]; then
      echo "temp directory was not created"
      exit 1
   else
      chmod a+rwx $tmpdir
   fi
fi

# Afanasy host name:
if [ -z $AF_HOSTNAME ]; then AF_HOSTNAME=`hostname`; fi

#
# PID file:
#
export pidfile="$tmpdir/$afapp.$AF_HOSTNAME.pid"

#
# LOG file:
#
export logfile="$tmpdir/$afapp.$AF_HOSTNAME.log"
logrotate() {
   log=$1
   if [ -f $log ]; then
      [ -f $log.8 ] && /bin/mv -f $log.8 $log.9
      [ -f $log.7 ] && /bin/mv -f $log.7 $log.8
      [ -f $log.6 ] && /bin/mv -f $log.6 $log.7
      [ -f $log.5 ] && /bin/mv -f $log.5 $log.6
      [ -f $log.4 ] && /bin/mv -f $log.4 $log.5
      [ -f $log.3 ] && /bin/mv -f $log.3 $log.4
      [ -f $log.2 ] && /bin/mv -f $log.2 $log.3
      [ -f $log.1 ] && /bin/mv -f $log.1 $log.2
      [ -f $log.0 ] && /bin/mv -f $log.0 $log.1
      /bin/mv -f $log $log.0
   fi
   return 0
}

execfile="$afroot/bin/$afapp"
startcmd="$afroot/init/afstart.sh $execfile $logfile"

function start(){
   echo "Starting $afapp..."
   if [ -f $pidfile ]; then
      kill `cat $pidfile`
      rm -fv $pidfile
   fi
   logrotate $logfile
   startcmd="$startcmd $pidfile"
   if [ "$UID" == "0" ] && [ ! -z "${nonrootuser}" ]; then
      su - $nonrootuser -c "$startcmd"
   else
      $startcmd
   fi
   if [ $? != 0 ]; then
      echo "Can't execute process."
      exit 1
   fi
   if [ ! -f $pidfile ]; then
      echo "Pid file was not created."
      exit 1
   fi
   echo $?
}

function stop(){
   echo "Stopping $afapp"
   if [ ! -f $pidfile ]; then
      echo "Application '$afapp.$AF_HOSTNAME' is not running (or pid file does not exist)."
      exit 1
   fi
   kill `cat $pidfile`
   if [ $? != 0 ]; then
      echo "Can't kill process."
      exit 1
   fi
   rm -f $pidfile
   if [ -f $pidfile ]; then
      echo "Can't delete pid file."
   fi
   echo $?
}

case $1 in
   start)
      start
      ;;

   stop)
      stop
      ;;

   restart)
      stop
      sleep 10
      start
      ;;

   launch)
      echo "Starting $afapp"
      if [ "$UID" == "0" ]; then
         echo "Error: Try not to launch under ROOT!"
         exit 1
      fi
      logrotate $logfile
      $startcmd
      ;;

   *)
      echo "Usage: $afapp {start|stop|restart|launch}"
      exit 1
      ;;
esac

exit 0

#!/bin/bash
#
# chkconfig: 2345 90 10
# description: Afanasy Daemon
#
# disable service name, as afrender and afserver uses links to the same file, and no doubles allowed
### BEGIN INIT INFO
##Provides: afanasy
# Required-Start:  $local_fs $remote_fs $network
# Required-Stop:   $local_fs $remote_fs $network
# Should-Start:    $syslog
# Should-Stop:     $syslog
# Default-Start:   2 3 4 5
# Default-Stop:    0 1 6
# Short-Description: Afanasy Daemon
### END INIT INFO

link=$0
# Check whether executed file is a link:
if [ ! -L $link ]; then
#   log_failure_msg "You should run a symbolic link named like application with absolute path to afdaemon script."
   echo "You should run a symbolic link named like application with absolute path to afdaemon script."
   exit 1
fi

# Get daemon location, solving link until get a file
cmd=$link
curdir=$PWD
cd `dirname $cmd`
while [ -L $cmd ]; do
   link=$cmd
   cmd=`readlink $link`
   cd `dirname $cmd`
done
cd $curdir

# Get Afanasy location:
curdir=$PWD
cd `dirname $cmd`
cd ..
afroot=$PWD
cd $curdir

# Get Afanasy application:
afapp=`basename $link`

# Afanasy "nonroot" user:
nonrootuser=renderer
if [ -f "${afroot}/nonrootuser" ]; then
   nonrootuser=`cat "${afroot}/nonrootuser"`
else
   echo "${nonrootuser}" > "${afroot}/nonrootuser"
fi
id "${nonrootuser}" || nonrootuser=""

# Temp directory:
tmpdir=/var/tmp/afanasy
if [ ! -d "$tmpdir" ]; then
   mkdir $tmpdir
   if [ ! -d "$tmpdir" ]; then
#      log_failure_msg "temp directory was not created"
      echo "temp directory was not created"
      exit 1
   else
      chmod a+rwx $tmpdir
   fi
fi

# Get lsb functions
. /lib/lsb/init-functions

# Afanasy host name:
if [ -z $AF_HOSTNAME ]; then AF_HOSTNAME=`hostname -s`; fi

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
#   log_begin_msg "Starting $afapp"
   echo "Starting $afapp"
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
#      log_failure_msg "Can't execute process."
      echo "Can't execute process."
      exit 1
   fi
   if [ ! -f $pidfile ]; then
#      log_failure_msg "Pid file was not created."
      echo "Pid file was not created."
      exit 1
   fi
#   log_end_msg $?
   echo $?
}

function stop(){
#   log_begin_msg "Stopping $afapp"
   echo "Stopping $afapp"
   if [ ! -f $pidfile ]; then
      log_failure_msg "Application '$afapp.$AF_HOSTNAME' is not running (or pid file does not exist)."
      exit 1
   fi
   kill `cat $pidfile`
   if [ $? != 0 ]; then
#      log_failure_msg "Can't kill process."
      echo "Can't kill process."
      exit 1
   fi
   rm -f $pidfile
   if [ -f $pidfile ]; then
#      log_failure_msg "Can't delete pid file."
      echo "Can't delete pid file."
   fi
#   log_end_msg $?
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

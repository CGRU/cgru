#!/bin/bash

sleep_sec=$1
[ -z "$sleep_sec" ] && sleep_sec=1

# Process parameters:
cleanup=0

# Cleanup argument check:
if [ "$1" == "c" ]; then
   echo "Cleanup mode:"
   cleanup=1
fi

# Temporary folder:
tmpdir=/tmp/afanasy_emulate
[ -d $tmpdir ] && rm -rf $tmpdir

# Cleanup previous logs:
outputdir=/var/tmp/afanasy
if [ -d $outputdir ]; then
   rm -rf $outputdir/tasksoutput/job_*
fi

# Cleanup mode:
if [ $cleanup == "1" ]; then
   rm -rf $outputdir/tasksoutput
   echo "Cleanup done."
   exit 0
fi

# Source CGRU setup:
if [ -z $CGRU_LOCATION ]; then
   pushd ../.. >> /dev/null
   source setup.sh
   popd >> /dev/null
fi

source $CGRU_LOCATION/utilities/python/setup.sh ""

# Create temporary folder:
mkdir $tmpdir

cd ..

JobsPack=10
Users=5
PausePeriod=36
PauseTime=12
Deletion=12
DeletionPausePeriod=9
DeletionPauseTime=11
counter=0
usr=$Users
pp=$PausePeriod
del=$Deletion
del_period=$DeletionPausePeriod
while [ 1 ]; do
   for(( jp=0; jp<$JobsPack; jp++)); do
      output="Job = $counter, user = $usr, pause = $pp, deletion = $del, period = $del_period"
      echo $output
      jobname="job_$counter"
      username="user_$usr"
      tmpfile=$tmpdir/$jobname
      echo $output > $tmpfile
      python ./job.py --name $jobname --user $username -b 2 -n 10 -p 3 --cmdpost "rm $tmpfile" > /dev/null
      if [ $? != 0 ]; then
         echo "Error creation new job, exiting."
         exit 1
      fi
      let usr=$usr-1
      [ $usr == 0 ] && usr=$Users
      let counter=$counter+1
   done
   let del=$del-1
   if [ $del == 0 ]; then
      echo "Deleting Jobs."
      $AF_ROOT/bin/afcmd jdel ".*"
      del=$Deletion
      let del_period=$del_period-1
      if [ $del_period == 0 ]; then
         echo "Deletion pause..."
         sleep $DeletionPauseTime
         del_period=$DeletionPausePeriod
      fi
   fi
   let pp=$pp-1
   if [ $pp == 0 ]; then
      echo "Pause..."
      pp=$PausePeriod
      sleep $PauseTime
   fi
   sleep $sleep_sec
done

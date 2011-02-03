#!/bin/bash

# Source CGRU setup:
if [ -z $CGRU_LOCATION ]; then
   pushd ../.. >> /dev/null
   source setup.sh
   popd >> /dev/null
fi

cd ..

# Create temporary folder:
tmpdir=/tmp/afanasy_emulate
[ -d $tmpdir ] && rm -rf $tmpdir
mkdir $tmpdir

# Cleanup previous logs:
outputdir=/var/tmp/afanasy
if [ -d $outputdir ]; then
   rm -rf $outputdir/jobs/job*
   rm -rf $outputdir/tasksoutput/job*
fi

Users=10
Deletion=14
j=0
u=$Users
d=$Deletion
while [ 1 ]; do
   output="Job = $j; User = $u; Deletion = $d"
   echo $output
   jobname="job$j"
   username="user$u"
   tmpfile=$tmpdir/$jobname
   echo $output > $tmpfile
   python ./job.py --name $jobname --user $username -b 2 -n 3 --cmdpost "rm $tmpfile"
   if [ $? != 0 ]; then
      echo "Error creation new job, exiting."
      exit 1
   fi
   let j=$j+1
   let u=$u-1
   let d=$d-1
   [ $u == 0 ] && u=$Users
   if [ $d == 0 ]; then
      echo "Deleting Jobs."
      $AF_ROOT/bin/afcmd jdel ".*"
      d=$Deletion
   fi
   sleep 1
done

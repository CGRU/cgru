#!/bin/bash

# Source CGRU setup:
if [ -z $CGRU_LOCATION ]; then
   pushd ../.. >> /dev/null
   source setup.sh
   popd >> /dev/null
fi

cd ..

tmpdir=/tmp/afanasy_emulate
[ -d $tmpdir ] || mkdir $tmpdir

Users=10
Deletion=100
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

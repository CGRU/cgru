#!/bin/bash

if [ "$1" == "-h" ]; then
    echo "Usage:"
    echo "-c                Cleanup mode."
    echo "--nocmdpost       Skip post command."
    exit 0
fi;

# Process parameters:
cleanup=0
JobsPack=10
Users=10
PausePeriod=1
PauseTime=10
DeletionPeriod=5
for arg in "$@"; do
    [ "$arg" == "--nocmdpost" ] && nocmdpost=1
    [ "$arg" == "-c" ] && cleanup=1
    JobsPack="$arg"
done
echo "JobsPack = $JobsPack"

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

cd ..

# Source CGRU setup:
if [ -z $CGRU_LOCATION ]; then
    pushd ../.. >> /dev/null
    source ./setup.sh
    popd >> /dev/null
fi

# Create temporary folder:
mkdir $tmpdir

counter=0
usr=$Users
pp=$PausePeriod
del=$DeletionPeriod
while [ 1 ]; do
    for(( jp=0; jp<$JobsPack; jp++)); do
        output="Job = $counter, user = $usr, pause = $pp, deletion = $del, period = $del_period"
        echo $output
        jobname="emulate_job_$counter"
        username="user_$usr"
        tmpfile=$tmpdir/$jobname

        if [ $jp == 0 ]; then
            python ./job.py --name $jobname --user $username -b 2 -n 10 -t 1 > /dev/null
            if [ $? != 0 ]; then
                echo "Error creation new job, exiting."
                exit 1
            fi
        elif [ -z "$nocmdpost" ]; then
            echo $output > $tmpfile
            python ./job.py --name $jobname --user $username -b 2 -n 10 -t 1 --cmdpost "rm $tmpfile" > /dev/null &
        else
            python ./job.py --name $jobname --user $username -b 2 -n 10 -t 1 > /dev/null &
        fi

        let usr=$usr-1
        [ $usr == 0 ] && usr=$Users
        let counter=$counter+1
    done
    sleep 1
    let pp=$pp-1
    if [ $pp == 0 ]; then
        pp=$PausePeriod
        let del=$del-1
        if [ $del == 0 ]; then
            echo "Deleting Jobs."
            del=$DeletionPeriod
            for(( dp=0; dp<$PauseTime; dp++)); do
                $AF_ROOT/bin/afcmd jdel "emulate_job_.*"
                sleep 1
            done
         else
            echo "Pause..."
            sleep $PauseTime
        fi
    fi
done

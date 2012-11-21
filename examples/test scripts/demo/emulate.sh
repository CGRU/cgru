#!/bin/bash

function usageExit(){
    echo "Usage:"
    echo "jobs + number    Number of jobs in each pack."
    echo "tasks + number   Number of tasks in each job."
    echo "nopost           Skip post command."
    echo "cleanup          Cleanup mode."
    exit 0
}

# Process parameters:
cleanup=0
JobsPack=10
Users=10
PausePeriod=1
PauseTime=10
DeletionPeriod=5
numtasks=10
while [ ! -z "$1" ]; do
	case "$1" in
	"jobs" )
		JobsPack=$2
		shift
		;;
	"tasks" )
		numtasks=$2
		shift
		;;
	"pause" )
		PauseTime=$2
		shift
		;;
	"nopost" )
		nocmdpost=1
		;;
	"cleanup" )
		cleanup=1
		;;
	"h" )
		usageExit
		;;
	*)
		echo "Unrecognized option $1"
		usageExit
		;;
	esac
	shift
done

echo "Jobs Pack    = $JobsPack"
echo "Tasks Number = $numtasks"
[ ! -z "$nocmdpost" ] && echo "No post command"

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
        output="Job = $counter, user = $usr, pause = $pp, deletion = $del"
        echo $output
        jobname="emulate_job_$counter"
        username="user_$usr"
        tmpfile=$tmpdir/$jobname

        if [ -z "$nocmdpost" ]; then
            echo $output > $tmpfile
            python ./job.py -s --name $jobname --user $username -b 2 -n $numtasks -t 1 --nonseq --cmdpost "rm $tmpfile" > /dev/null
        else
            python ./job.py -s --name $jobname --user $username -b 2 -n $numtasks -t 1 --nonseq > /dev/null
        fi

        if [ $? != 0 ]; then
            echo "Error creating a job, exiting."
            exit 1
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

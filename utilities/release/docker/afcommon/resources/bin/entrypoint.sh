#!/bin/bash
set -e

export CGRU_LOCATION=/opt/cgru

# Add CGRU bin to path:
export PATH="${CGRU_LOCATION}/bin:${PATH}"

# Add software to PATH:
export PATH="${CGRU_LOCATION}/software_setup/bin:${PATH}"

# Python module path:
export CGRU_PYTHON="${CGRU_LOCATION}/lib/python"

#########################
# AFANASY CONFIGURATION #
#########################

# Set Afanasy root:
export AF_ROOT=/opt/cgru/afanasy

export PATH="${AF_ROOT}/bin:${PATH}"

export AF_PYTHON="${AF_ROOT}/python"

export CGRU_PYTHONEXE="python3"

export PYTHONPATH="${CGRU_PYTHON}:${AF_PYTHON}"

/usr/local/bin/ep -v "/opt/cgru/afanasy/config_default.json"

if [ "${AF_SERVER_WAIT}x" == "yesx" ]
then 
	while ! ping -c 1 $AF_SERVERNAME 2> /dev/null
	do
		echo "Waiting for $AF_SERVERNAME ..."
		sleep 1
	done
fi

exec "$@"
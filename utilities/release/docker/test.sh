#!/bin/bash
set -e

pushd `pwd`> /dev/null
cd `dirname $0`

function launch {
	docker-compose -f test.yml $OPTS
}

export COMPOSE_PROJECT_NAME="cgru"
export CGRU_NETWORK="cgru"

if [ "$1x" == "bootstrapx" ]
then
	docker network create $CGRU_NETWORK
	cd afcommon
	docker build -t afcommon .
elif [ "$1x" == "x" ]
then
	OPTS="up -d"
	launch
else
	OPTS="$@"
	launch
fi

popd > /dev/null
#!/bin/bash
set -e

pushd "$PWD" > /dev/null
cd "$(dirname "$0")"

function launch {
	docker-compose -f test.yml "$OPTS"
}

export COMPOSE_PROJECT_NAME="cgru"

if [ "$1x" == "bootstrapx" ]
then
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

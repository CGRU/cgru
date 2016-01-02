#!/bin/bash

TOKENFILE="dockerhub.token"

pushd `pwd` > /dev/null

cd `dirname $0`

if [[ -r $TOKENFILE ]]
then
	TOKEN=`cat $TOKENFILE`
	echo "Token is $TOKEN"
else
	echo "Missing token file : $TOKENFILE"
	exit 1
fi

echo "Triggering ..."
curl -H "Content-Type: application/json" --data '{"build": true}' -X POST "https://registry.hub.docker.com/u/cgru/afcommon/trigger/${TOKEN}/"

popd > /dev/null
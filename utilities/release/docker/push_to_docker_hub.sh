#!/bin/bash
set -e

echo "Please input the version number to be pushed to docker hub"
read VERSION

CONTAINERS="afserver afrender"
DOCKER_HUB_USER="cgru"

TAGS="$VERSION latest"

docker login -u $DOCKER_HUB_USER

for CONTAINER in $CONTAINERS
do
	pushd `dirname $0` > /dev/null
	cd $CONTAINER
	docker build --no-cache -t $CONTAINER .
	popd > /dev/null
	for TAG in $TAGS
	do
		docker tag -f $CONTAINER $DOCKER_HUB_USER/$CONTAINER:$TAG
		docker push $DOCKER_HUB_USER/$CONTAINER:$TAG
	done
done
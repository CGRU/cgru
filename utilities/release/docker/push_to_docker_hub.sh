#!/bin/bash
set -e

CONTAINERS="$@"
DOCKER_HUB_USER="cgru"

docker login -u $DOCKER_HUB_USER

for CONTAINER in $CONTAINERS
do
	pushd `dirname $0` > /dev/null
	cd $CONTAINER
	CGRU_VERSION=`sed -n 's/^.*CGRU_VERSION *= *"\(.*\)".*/\1/p' Dockerfile`
	CGRU_CONTAINER_VERSION=`sed -n 's/^.*cgru_container_version *= *"\(.*\)".*/\1/p' Dockerfile`
	echo "Building $CONTAINER:$CGRU_VERSION-$CGRU_CONTAINER_VERSION"
	docker build --no-cache -t $DOCKER_HUB_USER/$CONTAINER:$CGRU_VERSION-$CGRU_CONTAINER_VERSION .
	docker tag -f $DOCKER_HUB_USER/$CONTAINER:$CGRU_VERSION-$CGRU_CONTAINER_VERSION $DOCKER_HUB_USER/$CONTAINER:$CGRU_VERSION
	docker tag -f $DOCKER_HUB_USER/$CONTAINER:$CGRU_VERSION $DOCKER_HUB_USER/$CONTAINER:latest
	popd > /dev/null

	for TAG in $DOCKER_HUB_USER/$CONTAINER:$CGRU_VERSION-$CGRU_CONTAINER_VERSION $DOCKER_HUB_USER/$CONTAINER:$CGRU_VERSION $DOCKER_HUB_USER/$CONTAINER:latest
	do
		echo "Pushing $TAG to $DOCKER_HUB_USER/"
		docker push $TAG
	done
done
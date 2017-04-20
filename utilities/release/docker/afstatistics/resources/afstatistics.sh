#!/bin/bash
set -e

while /opt/cgru/afanasy/bin/afcmd db_check 2>&1 | grep -q "NOT WORKING"
do
	echo "Waiting for database to be ready ..."
	sleep 1
done

echo "Checking Afanasy statistics DB structure ..."

/opt/cgru/afanasy/bin/afcmd db_updatetables 2>&1 \
 | egrep -q "relation.*does not exist" \
 && /opt/cgru/afanasy/bin/afcmd db_reset_all

echo "Launching web server ..."

rm -f /var/run/apache2/apache2.pid

. /etc/apache2/envvars

exec apache2 -DFOREGROUND
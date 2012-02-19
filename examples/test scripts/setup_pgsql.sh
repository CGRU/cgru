#!/bin/bash

pg_name="postgresql"
pg_ver="8.4"
pg_conf="/etc/${pg_name}/${pg_ver}/main/pg_hba.conf"
pg_init="/etc/init.d/${pg_name}-${pg_ver}"
pg_user="postgres"
af_init="/etc/init.d/afserver"
af_cmd="../../bin/afcmd"
db_name="afanasy"
db_user="afadmin"
db_passwd="AfPassword"

# Install PostgreSQL:
apt-get -y install $pg_name

# Check PostgreSQL init file:
if [ ! -f $pg_init ]; then
   echo "ERROR: File '$pg_init' not founded."
   exit 1
fi

# Check Afanasy server init file:
if [ -f $af_init ]; then
   $af_init stop
else
   echo "Warning: File '$af_init' not founded, install afanasy server if needed."
fi

# Stop PostgreSQL service:
$pg_init stop

# Check PostgreSQL config file:
if [ ! -f $pg_conf ]; then
   echo "ERROR: File '$pg_conf' not founded."
   exit 1
fi

# Add required line:
su - $pg_user -c "echo \"# Afanasy:\" >> $pg_conf; echo \"local afanasy afadmin password\" >> $pg_conf"

# Start PostgreSQL service:
$pg_init start

# Create database and user
su - $pg_user -c "createdb $db_name; psql -d $db_name -c \"CREATE USER ${db_user} PASSWORD '${db_passwd}';\""

# Check conection and create tables
if [ -f $af_cmd ]; then
   $af_cmd db_drivers
   $af_cmd db_check
   $af_cmd db_resetall
fi

# Start Afanasy server
[ -f $af_init ] && $af_init start

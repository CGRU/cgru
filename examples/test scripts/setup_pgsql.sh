#!/bin/bash

pg_conf=`find /etc/postgresql -name "pg_hba.conf"`
pg_user="postgres"
af_cmd="/opt/cgru/afanasy/bin/afcmd"
db_name="afanasy"
db_user="afadmin"
db_passwd="AfPassword"

# Install PostgreSQL:
apt-get -y install postgresql

# Stop services:
systemctl stop postgresql afserver

# Check PostgreSQL config file:
if [ ! -f $pg_conf ]; then
   echo "ERROR: File '$pg_conf' not found."
   exit 1
fi

# Add required config line to the PostgreSQL configuration, if not yet exists
afanasyConfigGrep=$(grep 'Afanasy Database' $pg_conf)
if [[ "$afanasyConfigGrep" == "" ]]; then
  su - $pg_user -c "echo \"\" >> $pg_conf"
  su - $pg_user -c "echo \"##### Afanasy Database ######\" >> $pg_conf"
  su - $pg_user -c "echo \"local afanasy afadmin password\" >> $pg_conf"
fi

# Start PostgreSQL service:
systemctl start postgresql

# Create database and user
su - $pg_user -c "createdb $db_name;"
su - $pg_user -c "psql -d $db_name -c \"CREATE USER ${db_user} PASSWORD '${db_passwd}';\""

# Check connection and create tables
if [ -f $af_cmd ]; then
   # $af_cmd db_drivers # not present anymore?
   $af_cmd db_check
   $af_cmd db_reset_all
fi

# Start Afanasy service:
systemctl start afserver

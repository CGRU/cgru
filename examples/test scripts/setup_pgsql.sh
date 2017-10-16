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

# Add required config line to the PostgreSQL configuration:
su - $pg_user -c "echo \"# Afanasy:\" >> $pg_conf; echo \"local afanasy afadmin password\" >> $pg_conf"

# Start PostgreSQL service:
systemctl start postgresql

# Create database and user
su - $pg_user -c "psql -d $db_name -c \"CREATE DATABASE IF NOT EXISTS ${db_name};\""
su - $pg_user -c "psql -d $db_name -c \"CREATE USER ${db_user} PASSWORD '${db_passwd}';\""

# Check connection and create tables
if [ -f $af_cmd ]; then
   # $af_cmd db_drivers # not present anymore?
   $af_cmd db_check
   $af_cmd db_reset_all
fi

# Start Afanasy service:
systemctl start afserver

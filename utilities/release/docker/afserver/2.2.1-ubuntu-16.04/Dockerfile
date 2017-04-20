FROM cgru/afcommon:2.2.1-ubuntu-16.04

LABEL maintainer="Alexandre Buisine <alexandrejabuisine@gmail.com>"
LABEL cgru_container_version="1.0.0"

USER root
RUN DEBIAN_FRONTEND=noninteractive apt-get -qq update \
 && apt-get -yqq install \
    postgresql-client \
 && apt-get -yqq clean \
 && rm -rf /var/lib/apt/lists/*

RUN sed -i 's/"af_db_conninfo":".\+",/"af_db_conninfo":"host=${AF_DB_HOST} dbname=${AF_DB_NAME} user=${AF_DB_USER} password=${AF_DB_PASSWORD}",/' /opt/cgru/afanasy/config_default.json

ENV AF_DB_HOST="db" \
 AF_DB_NAME="afanasy" \
 AF_DB_USER="afadmin" \
 AF_DB_PASSWORD="AfPassword"

EXPOSE 51000
USER render

CMD ["/opt/cgru/afanasy/bin/afserver"]
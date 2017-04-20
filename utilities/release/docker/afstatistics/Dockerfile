FROM cgru/afcommon:2.2.1-ubuntu-14.04

LABEL maintainer="Alexandre Buisine <alexandrejabuisine@gmail.com>"
LABEL cgru_container_version="1.0.0"

USER root
RUN DEBIAN_FRONTEND=noninteractive apt-get -qq update \
 && apt-get -yqq install \
    apache2 \
    libapache2-mod-php5 \
    php5-pgsql \
 && apt-get -yqq clean \
 && rm -rf /var/lib/apt/lists/*

COPY resources/afstatistics.sh /usr/local/bin/
COPY resources/000-afstatistics.conf /etc/apache2/sites-available

RUN chmod -R +x /usr/local/bin/* \
 && a2dissite 000-default \
 && a2ensite 000-afstatistics \
 && sed -ri \
	-e 's!^(\s*ErrorLog)\s+\S+!\1 /proc/self/fd/2!g' \
	/etc/apache2/apache2.conf \
 && chown -R www-data /opt/cgru/afanasy/statistics \
 && sed -i 's/"af_db_conninfo":".\+",/"af_db_conninfo":"host=${AF_DB_HOST} dbname=${AF_DB_NAME} user=${AF_DB_USER} password=${AF_DB_PASSWORD}",/' /opt/cgru/afanasy/config_default.json \
 && sed -i "s/'host=localhost dbname=afanasy user=afadmin password=AfPassword'/'host=' . getenv('AF_DB_HOST') . ' dbname=' . getenv('AF_DB_NAME') . ' user=' . getenv('AF_DB_USER') . ' password=' . getenv('AF_DB_PASSWORD')/" /opt/cgru/afanasy/statistics/server.php

ENV AF_DB_HOST="db" \
 AF_DB_NAME="afanasy" \
 AF_DB_USER="afadmin" \
 AF_DB_PASSWORD="AfPassword" \
 AF_SERVER_WAIT="yes"
ENV AF_SERVERNAME=${AF_DB_HOST}

EXPOSE 80

CMD ["/usr/local/bin/afstatistics.sh"]
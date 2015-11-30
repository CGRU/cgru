FROM ubuntu:14.04

STOPSIGNAL SIGINT

ADD http://downloads.sourceforge.net/project/cgru/2.0.7/cgru.2.0.7.ubuntu14.04.3_amd64.tar.gz /tmp
WORKDIR /tmp
RUN apt-get -qq update \
 && apt-get -y install \
 	libpq5 \
 	libpython2.7 \
 && apt-get -yqq clean \
 && rm -rf /var/lib/apt/lists/* \
 && tar xzvf cgru*.tar.gz \
 && dpkg -i /tmp/cgru-common*.deb /tmp/afanasy-common*.deb /tmp/afanasy-render*.deb \
 && rm -r /tmp/*
 
COPY resources/config.json /opt/cgru/

WORKDIR /opt/cgru
VOLUME /var/tmp/afanasy
CMD ["start/AFANASY/_afrender.sh"]
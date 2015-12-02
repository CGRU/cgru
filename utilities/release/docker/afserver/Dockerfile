FROM ubuntu:14.04
MAINTAINER Alexandre Buisine <alexandrejabuisine@gmail.com>
ENV CGRU_VERSION="2.0.7"
LABEL cgru_version=$CGRU_VERSION cgru_container_version="1.0.0"

STOPSIGNAL SIGINT

ADD http://downloads.sourceforge.net/project/cgru/$CGRU_VERSION/cgru.$CGRU_VERSION.ubuntu14.04.3_amd64.tar.gz /tmp
WORKDIR /tmp
RUN apt-get -qq update \
 && apt-get -y install \
 	libpq5 \
 	libpython2.7 \
 	python3-sip \
 	tcpdump \
 && apt-get -yqq clean \
 && rm -rf /var/lib/apt/lists/* \
 && tar xzvf cgru*.tar.gz \
 && dpkg -i /tmp/cgru-common*.deb /tmp/afanasy-common*.deb /tmp/afanasy-server*.deb \
 && rm -r /tmp/*
 
COPY resources/bin/ /usr/local/bin/
RUN chmod -R +rx /usr/local/bin/ /usr/local/sbin/

VOLUME /var/tmp/afanasy
EXPOSE 51000
ENV AF_USERNAME="render"
# ENV AF_HOSTNAME=""

WORKDIR /opt/cgru
USER render
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["/opt/cgru/afanasy/bin/afserver"]
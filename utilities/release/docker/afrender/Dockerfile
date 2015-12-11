FROM ubuntu:14.04
MAINTAINER Alexandre Buisine <alexandrejabuisine@gmail.com>
ENV CGRU_VERSION="2.0.8"
LABEL cgru_version=$CGRU_VERSION cgru_container_version="1.0.0"

STOPSIGNAL SIGINT

WORKDIR /tmp
RUN apt-get -qq update \
 && apt-get -y install \
 	libpq5 \
 	libpython2.7 \
 	python3-sip \
 	tcpdump \
 && apt-get -yqq clean \
 && rm -rf /var/lib/apt/lists/*
 
ADD http://downloads.sourceforge.net/project/cgru/$CGRU_VERSION/cgru.$CGRU_VERSION.ubuntu14.04.3_amd64.tar.gz ./
RUN tar xzvf cgru*.tar.gz \
 && dpkg -i cgru-common*.deb afanasy-common*.deb afanasy-render*.deb \
 && rm -rf *
 
ADD https://github.com/kreuzwerker/envplate/releases/download/v0.0.8/ep-linux /usr/local/bin/ep
COPY resources/config.json /home/render/.cgru/config.json
COPY resources/bin/ /usr/local/bin/
RUN chmod -R +rx /usr/local/bin/ /usr/local/sbin/ && chown -R render:render /home/render/
VOLUME /var/tmp/afanasy
EXPOSE 51001
ENV AF_USERNAME="render" AF_SERVERNAME="afrender"
# ENV AF_HOSTNAME=""

WORKDIR /opt/cgru
USER render
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["/opt/cgru/afanasy/bin/afrender"]
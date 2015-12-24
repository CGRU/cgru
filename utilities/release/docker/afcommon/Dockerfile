FROM ubuntu:14.04
MAINTAINER Alexandre Buisine <alexandrejabuisine@gmail.com>
ENV CGRU_VERSION="2.0.8"
LABEL cgru_version=$CGRU_VERSION cgru_container_version="1.0.0"

STOPSIGNAL SIGINT

RUN DEBIAN_FRONTEND=noninteractive apt-get -qq update \
 && apt-get -yqq install \
 	libpq5 \
 	libpython2.7 \
 	python3-sip \
 	tcpdump \
 	telnet \
 	imagemagick \
 && apt-get -yqq clean \
 && rm -rf /var/lib/apt/lists/*
 
WORKDIR /tmp
ADD http://downloads.sourceforge.net/project/cgru/$CGRU_VERSION/cgru.$CGRU_VERSION.ubuntu14.04.3_amd64.tar.gz ./
RUN tar xzvf cgru*.tar.gz \
 && dpkg -i cgru-common*.deb afanasy-common*.deb afanasy-server*.deb afanasy-render*.deb \
 && rm -rf *
 
ADD https://github.com/kreuzwerker/envplate/releases/download/v0.0.8/ep-linux /usr/local/bin/ep
COPY resources/config.json /tmp/config.json
COPY resources/bin/ /usr/local/bin/
RUN chmod -R +rx /usr/local/bin/ /usr/local/sbin/ \
 && mkdir /root/.cgru/ \
 && cp /tmp/config.json /root/.cgru/config.json \
 && mv /tmp/config.json /home/render/.cgru/config.json \
 && chmod -R a+r,u+Xw /root/.cgru /home/render/.cgru \
 && chown -R render:render /home/render/.cgru

VOLUME /var/tmp/afanasy
ENV AF_USERNAME="render" \
 AF_SERVERNAME="afrender" \
 AF_SERVER_WAIT="no" \
 AF_SERVERPORT=51000 \
 AF_CLIENTPORT=51001 \
 AF_RENDER_CMD_REBOOT="killall -s SIGINT afrender" \
 AF_RENDER_CMD_SHUTDOWN="killall -s SIGINT afrender" \
 AF_RENDER_CMD_WOLSLEEP="killall -s SIGINT afrender" \
 AF_THUMBNAIL_EXTENSIONS="[\"exr\",\"dpx\",\"jpg\",\"jpeg\",\"png\",\"tif\",\"tiff\",\"tga\"]" \
 AF_THUMBNAIL_CMD="convert -identify '%(image)s' %(pre_args)s -thumbnail '100x100^' -gravity center -extent 100x100 -colorspace sRGB '%(thumbnail)s'"
# ENV AF_HOSTNAME=""

WORKDIR /opt/cgru
USER render
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["/opt/cgru/afanasy/bin/afcmd"]
FROM ubuntu:14.04

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

LABEL maintainer="Alexandre Buisine <alexandrejabuisine@gmail.com>"

ENV CGRU_VERSION="2.1.0" CGRU_UBUNTU_VERSION="14.04.4"
LABEL cgru_version=$CGRU_VERSION cgru_container_version="1.0.0"

WORKDIR /tmp
ADD http://downloads.sourceforge.net/project/cgru/${CGRU_VERSION}/cgru.${CGRU_VERSION}.ubuntu${CGRU_UBUNTU_VERSION}_amd64.tar.gz ./cgru.${CGRU_VERSION}.ubuntu${CGRU_UBUNTU_VERSION}_amd64.tar.gz
RUN tar xzvf cgru*.tar.gz \
 && dpkg -i cgru-common*.deb afanasy-common*.deb afanasy-server*.deb afanasy-render*.deb \
 && rm -rf *

ADD https://github.com/kreuzwerker/envplate/releases/download/v0.0.8/ep-linux /usr/local/bin/ep
COPY docker-entrypoint.sh /
RUN chmod +rx /usr/local/bin/ep /docker-entrypoint.sh \
 && chmod 666 /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_servername":".\+",/"af_servername":"${AF_SERVERNAME}",/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_serverport":[0-9]\+,/"af_serverport":${AF_SERVERPORT},/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_clientport":[0-9]\+,/"af_clientport":${AF_CLIENTPORT},/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_render_cmd_reboot":".\+",/"af_render_cmd_reboot":"${AF_RENDER_CMD_REBOOT}",/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_render_cmd_shutdown":".\+",/"af_render_cmd_shutdown":"${AF_RENDER_CMD_SHUTDOWN}",/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_render_cmd_wolsleep":".\+",/"af_render_cmd_wolsleep":"${AF_RENDER_CMD_WOLSLEEP}",/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_thumbnail_extensions":.\+,/"af_thumbnail_extensions":${AF_THUMBNAIL_EXTENSIONS},/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_thumbnail_cmd":".\+",/"af_thumbnail_cmd":"${AF_THUMBNAIL_CMD}",/' /opt/cgru/afanasy/config_default.json

VOLUME /var/tmp/afanasy
ENV AF_USERNAME="render" \
 AF_SERVERNAME="afrender" \
 AF_SERVER_WAIT="no" \
 AF_SERVERPORT=51000 \
 AF_RENDER_CMD_REBOOT="killall -s SIGINT afrender" \
 AF_RENDER_CMD_SHUTDOWN="killall -s SIGINT afrender" \
 AF_RENDER_CMD_WOLSLEEP="killall -s SIGINT afrender" \
 AF_THUMBNAIL_EXTENSIONS="[\"exr\",\"dpx\",\"jpg\",\"jpeg\",\"png\",\"tif\",\"tiff\",\"tga\"]" \
 AF_THUMBNAIL_CMD="convert -identify '%(image)s' %(pre_args)s -thumbnail '100x100^' -gravity center -extent 100x100 -colorspace sRGB '%(thumbnail)s'"
# ENV AF_HOSTNAME=""

WORKDIR /opt/cgru
USER render
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["/opt/cgru/afanasy/bin/afcmd"]
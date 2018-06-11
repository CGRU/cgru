FROM cgru/afcommon:2.2.3-ubuntu1604

LABEL maintainer="Alexandre Buisine <alexandrejabuisine@gmail.com>" version="1.0.0"

COPY resources/docker-entrypoint-afrender.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-entrypoint-afrender.sh \
 && sed -i 's/"af_render_cmd_reboot":".\+",/"af_render_cmd_reboot":"${AF_RENDER_CMD_REBOOT}",/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_render_cmd_shutdown":".\+",/"af_render_cmd_shutdown":"${AF_RENDER_CMD_SHUTDOWN}",/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_render_cmd_wolsleep":".\+",/"af_render_cmd_wolsleep":"${AF_RENDER_CMD_WOLSLEEP}",/' /opt/cgru/afanasy/config_default.json \
 && sed -i 's/"af_render_default_capacity":.\+,/"af_render_default_capacity":${AF_RENDER_DEFAULT_CAPACITY},/' /opt/cgru/afanasy/config_default.json

ENV AF_RENDER_CMD_REBOOT="killall -s SIGINT afrender" \
 AF_RENDER_CMD_SHUTDOWN="killall -s SIGINT afrender" \
 AF_RENDER_CMD_WOLSLEEP="killall -s SIGINT afrender" \
 AF_RENDER_DEFAULT_CAPACITY=1000 \
 AF_RENDER_AUTO_CAPACITY_PER_PROC=500

USER render
ENTRYPOINT ["/usr/local/bin/docker-entrypoint-afrender.sh"]
CMD ["/opt/cgru/afanasy/bin/afrender"]
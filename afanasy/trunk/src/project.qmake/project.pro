TEMPLATE = subdirs
unix:SUBDIRS = libafanasy libafnetwork libafsql server libafapi cmd libafqt render talk watch monitor libpyaf
win32:SUBDIRS = libafanasy libafapi libafqt render talk watch monitor libpyaf

libafnetwork.depends = libafanasy
libafsql.depends = libafanasy
server.depends = libafanasy libafnetwork libafsql
libafapi.depends = libafanasy
cmd.depends = libafanasy libafnetwork libafsql libafapi
libafqt.depends = libafanasy
render.dependes = libafqt
talk.depends = libafqt
watch.depends = libafqt
monitor.depends = libafqt
libpyaf.depends = libafanasy

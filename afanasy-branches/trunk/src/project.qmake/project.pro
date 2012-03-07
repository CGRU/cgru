TEMPLATE = subdirs
unix:SUBDIRS = libafanasy libafsql server libafapi cmd libafqt render talk watch monitor libpyaf
win32:SUBDIRS = libafanasy libafapi libafqt render talk watch monitor libpyaf

libafsql.depends = libafanasy
server.depends = libafanasy libafsql
libafapi.depends = libafanasy
cmd.depends = libafanasy libafsql libafapi
libafqt.depends = libafanasy
render.depends = libafqt
talk.depends = libafqt
watch.depends = libafqt
monitor.depends = libafqt
libpyaf.depends = libafanasy

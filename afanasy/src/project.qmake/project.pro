TEMPLATE = subdirs
SUBDIRS = libafanasy libafsql server cmd libafqt render talk watch monitor

libafsql.depends = libafanasy
server.depends = libafanasy libafsql
cmd.depends = libafanasy libafsql
libafqt.depends = libafanasy
render.depends = libafqt
talk.depends = libafqt
watch.depends = libafqt
monitor.depends = libafqt
libpyaf.depends = libafanasy

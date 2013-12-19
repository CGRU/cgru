TEMPLATE = subdirs
SUBDIRS = libafanasy libafsql server cmd libafqt render watch

libafsql.depends = libafanasy
server.depends = libafanasy libafsql
cmd.depends = libafanasy libafsql
libafqt.depends = libafanasy
render.depends = libafqt
watch.depends = libafqt
libpyaf.depends = libafanasy

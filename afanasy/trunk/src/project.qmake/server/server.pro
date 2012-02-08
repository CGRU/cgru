TARGET = afserver
DESTDIR = ../../../bin

TEMPLATE = app

CONFIG += thread release

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../server
LIBS += -lpq
LIBS += -L../libafsql -lafsql
LIBS += -L../libafnetwork -lafnetwork
LIBS += -L../libafanasy -lafanasy
LIBS += $$(AF_PYTHON_LIB)

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

win32: DEFINES += WINNT
unix {
   DEFINES += UNIX
   macx {
      DEFINES += MACOSX
   } else {
      DEFINES += LINUX
   }
}

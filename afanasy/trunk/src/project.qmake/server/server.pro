TARGET = afserver
DESTDIR = ../../../bin

TEMPLATE = app

CONFIG += qt thread release
QT -= gui
QT += sql

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../server
LIBS += -L../libafsql -lafsql
LIBS += -L../libafnetwork -lafnetwork
LIBS += -L../libafanasy -lafanasy
LIBS += $$(AF_PYTHON_LIB)

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

win32: DEFINES += WINNT
unix {
   macx {
      DEFINES += MACOSX
   } else {
      DEFINES += LINUX
   }
}

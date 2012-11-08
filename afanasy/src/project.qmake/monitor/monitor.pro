TARGET = afmonitor
DESTDIR = ../../../bin

TEMPLATE = app

CONFIG += qt release
QT += network

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../monitor
LIBS += -L../libafqt -lafqt
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

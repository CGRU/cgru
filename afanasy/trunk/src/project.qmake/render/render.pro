TARGET = afrender
DESTDIR = ../../../bin

TEMPLATE = app

CONFIG += thread release

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../render
LIBS += -L../libafanasy -lafanasy
LIBS += -framework CoreFoundation
LIBS += -framework IOKit
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

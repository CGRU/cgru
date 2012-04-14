TARGET = afcmd
DESTDIR = ../../../bin

TEMPLATE = app

CONFIG += release

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)
QMAKE_CXXFLAGS += -I/usr/include
QMAKE_CXXFLAGS += -I/usr/include/postgresql

DIR = ../../cmd
LIBS += -lpq
LIBS += -L../libafsql -lafsql
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

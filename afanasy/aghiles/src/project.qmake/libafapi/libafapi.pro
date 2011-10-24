TARGET = afapi

TEMPLATE = lib
CONFIG = staticlib

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../libafapi

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

TARGET = afanasy

TEMPLATE = lib
CONFIG = staticlib

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../libafanasy

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

DEFINES += "CGRU_REVISION=$$(CGRU_REVISION)"

win32: DEFINES += WINNT
unix {
   DEFINES += UNIX
   macx {
      DEFINES += MACOSX
   } else {
      DEFINES += LINUX
   }
}

DIR = ../../libafanasy/passwd
SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

DIR = ../../libafanasy/msgclasses
SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

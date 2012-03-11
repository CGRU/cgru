TARGET = afqt

TEMPLATE = lib
CONFIG = staticlib

CONFIG += qt
QT -= gui
QT += network

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../libafqt

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

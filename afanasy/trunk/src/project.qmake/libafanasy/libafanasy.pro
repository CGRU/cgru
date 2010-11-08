TARGET = afanasy

TEMPLATE = lib
CONFIG = staticlib

CONFIG += qt
QT -= gui

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../libafanasy

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

DEFINES += "CGRU_VERSION=$$(CGRU_VERSION)"

win32:DEFINES += WINNT
macx:DEFINES += MACOSX
unix:DEFINES += LINUX

DIR = ../../libafanasy/passwd
SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

DIR = ../../libafanasy/msgclasses
SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

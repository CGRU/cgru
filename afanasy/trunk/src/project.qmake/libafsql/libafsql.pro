TARGET = afsql

TEMPLATE = lib
CONFIG = staticlib

CONFIG += qt
QT -= gui
QT += sql

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../libafsql

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

win32:DEFINES += WINNT
macx:DEFINES += MACOSX
unix:DEFINES += LINUX

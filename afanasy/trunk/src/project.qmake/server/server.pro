TARGET = afserver
DESTDIR = ../../../bin

TEMPLATE = app

CONFIG += thread
CONFIG += qt
QT -= gui
QT += xml network sql

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)
LIBS += $$(AF_PYTHON_LIB)

DIR = ../../server
LIBS += -L../libafsql -lafsql
LIBS += -L../libafnetwork -lafnetwork
LIBS += -L../libafanasy -lafanasy

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

win32:DEFINES += WINNT
macx:DEFINES += MACOSX
unix:DEFINES += LINUX

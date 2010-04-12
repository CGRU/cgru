TARGET = afcmd
DESTDIR = ../../../bin

TEMPLATE = app

CONFIG += qt
QT -= gui
QT += xml network sql

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../cmd
LIBS += -L../libafapi -lafapi
LIBS += -L../libafsql -lafsql
LIBS += -L../libafnetwork -lafnetwork
LIBS += -L../libafanasy -lafanasy
LIBS += $$(AF_PYTHON_LIB)

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

win32:DEFINES += WINNT
macx:DEFINES += MACOSX
unix:DEFINES += LINUX

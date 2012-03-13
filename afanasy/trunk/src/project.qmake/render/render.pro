TARGET = afreder
DESTDIR = ../../../bin

TEMPLATE = app

CONFIG += qt
QT -= gui
QT += xml network

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)
LIBS += $$(AF_PYTHON_LIB)

DIR = ../../render
LIBS += -L../libafqt -lafqt
LIBS += -L../libafanasy -lafanasy

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

win32:DEFINES += WINNT
macx:DEFINES += MACOSX
unix:DEFINES += LINUX

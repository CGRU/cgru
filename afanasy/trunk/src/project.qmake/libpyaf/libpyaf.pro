TARGET = pyaf

TEMPLATE = lib
CONFIG = dll

CONFIG += qt
QT -= gui
QT += xml network

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../libpyaf
LIBS += -L../libafapi -lafapi
LIBS += -L../libafanasy -lafanasy

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)
LIBS += $$(AF_PYTHON_LIB)

win32:DEFINES += WINNT
macx:DEFINES += MACOSX
unix:DEFINES += LINUX

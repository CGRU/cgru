TARGET = afnetwork

TEMPLATE = lib
CONFIG = staticlib

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../libafnetwork

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

win32:DEFINES += WINNT
macx:DEFINES += MACOSX
unix:DEFINES += LINUX

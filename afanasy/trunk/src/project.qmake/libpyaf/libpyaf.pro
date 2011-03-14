TARGET = pyaf

TEMPLATE = lib
CONFIG = dll

QMAKE_CXXFLAGS += $$(AF_PYTHON_INC)

DIR = ../../libpyaf
LIBS += -L../libafapi -lafapi
LIBS += -L../libafanasy -lafanasy
LIBS += $$(AF_PYTHON_LIB)
LIBS += $$(AF_PYTHON_MODULE_EXTRA_LIBS)

SOURCES += $$system(ls $$DIR/*.cpp)
HEADERS += $$system(ls $$DIR/*.h)

DEFINES += "CGRU_REVISION=$$(CGRU_REVISION)"

win32:DEFINES += WINNT
macx:DEFINES += MACOSX
unix:DEFINES += LINUX

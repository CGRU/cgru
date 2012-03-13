file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml QtGui REQUIRED)

include(${QT_USE_FILE})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")
include_directories("${afanasy_SOURCE_DIR}/libafanasy/src")
include_directories("${afanasy_SOURCE_DIR}/libafqt/src")

set(moc_hdr
	"${CMAKE_CURRENT_SOURCE_DIR}/src/actionid.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/buttonmonitor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/colorwidget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ctrlrenders.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ctrlsortfilter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/dialog.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fontwidget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/modelnodes.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/numberwidget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/listitems.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/listjobs.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/listmonitors.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/listnodes.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/listrenders.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/listtalks.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/listtasks.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/listusers.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/wndcustomizegui.h"
)

qt4_wrap_cpp(moc_src ${moc_hdr})

add_executable(afwatch ${src} ${inc} ${moc_src})
target_link_libraries(afwatch afqt)
install(TARGETS afwatch RUNTIME DESTINATION bin)

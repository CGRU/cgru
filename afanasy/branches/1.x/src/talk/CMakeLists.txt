file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml QtGui REQUIRED)

include(${QT_USE_FILE})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")
include_directories("${afanasy_SOURCE_DIR}/libafanasy/src")
include_directories("${afanasy_SOURCE_DIR}/libafqt/src")

set(moc_hdr
	"${CMAKE_CURRENT_SOURCE_DIR}/src/dialog.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/editor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/userslist.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/textview.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/tray.h"
)

qt4_wrap_cpp(moc_src ${moc_hdr})

add_executable(aftalk ${src} ${inc} ${moc_src})
target_link_libraries(aftalk afqt)
install(TARGETS aftalk RUNTIME DESTINATION bin)

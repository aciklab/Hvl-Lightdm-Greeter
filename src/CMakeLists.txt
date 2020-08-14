set(PROJECT hvl-lightdm-greeter)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(LIGHTDM_QT_INCLUDE_DIRS /usr/include/lightdm-qt5-3)



#link_directories(/usr/lib/x86_64-linux-gnu)

file(GLOB SRCS *.cpp *ui *moc)
message(STATUS "SRCS ${SRCS}")

find_package (Threads REQUIRED)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools)
find_package(Qt5X11Extras REQUIRED)
find_package(Qt5 COMPONENTS Core Quick REQUIRED)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)



add_definitions(-DUNIX)
add_definitions(-D_LIB)
add_definitions(-DNDEBUG)

QT5_ADD_RESOURCES(RSCS ../hvl-lightdm-greeter.qrc)

include(FindPkgConfig)
pkg_check_modules(LIGHTDM_QT liblightdm-qt5-3)

include_directories ( ${CMAKE_CURRENT_SOURCE_DIR}
                      ${CMAKE_CURRENT_BINARY_DIR}
                      ${LIGHTDM_QT_INCLUDE_DIRS}
)


file(GLOB TS_FILES ../languages/*.ts)
file(GLOB QML_FILES ../ui/*.qml)

qt5_add_translation(QM_FILES ${TS_FILES})

add_custom_target(translations DEPENDS ${QM_FILES})


add_executable (hvl-lightdm-greeter ${SRCS} ${RSCS} ${QM_FILES} ${QML_FILES})
#add_executable(${PROJECT_NAME} "main.cpp" "qml.qrc")

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries ( hvl-lightdm-greeter Qt5::Widgets Qt5::Network lightdm-qt5-3 X11 Xcursor Xrandr Qt5X11Extras Qt5::Core Qt5::Quick)


install(TARGETS ${PROJECT} RUNTIME DESTINATION bin)
install(FILES ${QM_FILES} DESTINATION /usr/share/lightdm/lightdm-hvl-greeter.conf.d/lang)

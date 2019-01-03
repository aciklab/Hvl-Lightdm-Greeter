cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
set(PROJECT hvl-lightdm-greeter)
project(${PROJECT})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(LIGHTDM_QT_LIBRARIES /usr/lib/x86_64-linux-gnu/liblightdm-qt5-3.so)
set(LIGHTDM_QT_INCLUDE_DIRS /usr/include/lightdm-qt5-3)
set(LIGHTDM_X11_LIBRARIES /usr/lib/x86_64-linux-gnu/libX11.so)
set(LIGHTDM_X11_EXTRA_LIBRARIES /usr/lib/x86_64-linux-gnu/libQt5X11Extras.so)
set(LIGHTDM_XCURSOR_LIBRARIES /usr/lib/x86_64-linux-gnu/libXcursor.so)



file(GLOB SRCS *.cpp *ui *moc)
message(STATUS "SRCS ${SRCS}")

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineCore REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)
find_package(Qt5LinguistTools)
find_package(Qt5X11Extras REQUIRED)



#find_package(Qt5X11Extras REQUIRED QUIET)

QT5_ADD_RESOURCES(RSCS hvl-lightdm-greeter.qrc)

file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/languages/*.ts)

qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})


include(FindPkgConfig)
pkg_check_modules(LIGHTDM_QT liblightdm-qt5-3)

include_directories ( ${CMAKE_CURRENT_SOURCE_DIR}
                      ${CMAKE_CURRENT_BINARY_DIR}
                      ${LIGHTDM_QT_INCLUDE_DIRS}
)

add_executable ( hvl-lightdm-greeter ${SRCS} ${RSCS} ${QM_FILES})




target_link_libraries ( hvl-lightdm-greeter Qt5::Widgets Qt5::Network Qt5::WebEngine Qt5::WebEngineCore Qt5::Quick Qt5::WebEngineWidgets ${LIGHTDM_QT_LIBRARIES} ${LIGHTDM_X11_LIBRARIES} ${LIGHTDM_CAIRO_LIBRARIES}
    ${LIGHTDM_X11_EXTRA_LIBRARIES} ${LIGHTDM_XCURSOR_LIBRARIES})

install(TARGETS ${PROJECT} RUNTIME DESTINATION bin)
install(FILES  hvl-lightdm-greeter.desktop  DESTINATION /usr/share/xgreeters)

if (NOT KEEP_CONFIGS)
    install(FILES hvl-lightdm-greeter.conf DESTINATION /usr/share/lightdm/lightdm-hvl-greeter.conf.d)
    install(FILES ${QM_FILES} DESTINATION /usr/share/lightdm/lightdm-hvl-greeter.conf.d/lang)
endif()


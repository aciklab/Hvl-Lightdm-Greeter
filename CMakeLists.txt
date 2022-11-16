cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
project(hvl-lightdm-greeter)
add_subdirectory(src)
find_package(Qt5 5.11 REQUIRED COMPONENTS Core)

install(FILES  hvl-lightdm-greeter.desktop  DESTINATION /usr/share/xgreeters)
install(FILES hvl-lightdm-greeter.conf DESTINATION /usr/share/lightdm/lightdm-hvl-greeter.conf.d)


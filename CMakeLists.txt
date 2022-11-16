cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
project(hvl-lightdm-greeter)
add_subdirectory(src)

install(FILES  hvl-lightdm-greeter.desktop  DESTINATION /usr/share/xgreeters)
install(FILES hvl-lightdm-greeter.conf DESTINATION /usr/share/lightdm/lightdm-hvl-greeter.conf.d)


#!/bin/bash


mkdir -p release/
cp -r DEBIAN ./release
mkdir -p release/usr/share/lightdm/lightdm-hvl-greeter.conf.d/
mkdir -p release/usr/share/lightdm/lightdm-hvl-greeter.conf.d/lang
mkdir -p release/usr/share/lightdm/lightdm-hvl-greeter.conf.d/resources/bgs
mkdir -p release/usr/local/bin/
mkdir -p release/usr/share/doc
mkdir -p release/usr/share/doc/lightdm-hvl-greeter
mkdir -p release/usr/share/xgreeters

mkdir build && cd build
cmake ..
make
cd ..

cp build/src/tr.qm release/usr/share/lightdm/lightdm-hvl-greeter.conf.d/lang
cp build/src/hvl-lightdm-greeter release/usr/local/bin/
cp src/resources/bgs/bg1.jpg release/usr/share/lightdm/lightdm-hvl-greeter.conf.d/resources/bgs
cp LICENSE release/usr/share/doc/lightdm-hvl-greeter/copyright
cp hvl-lightdm-greeter.desktop release/usr/share/xgreeters/
cp hvl-lightdm-greeter.conf release/usr/share/lightdm/lightdm-hvl-greeter.conf.d/hvl-lightdm-greeter.conf.tmp

chmod 644 release/usr/share/lightdm/lightdm-hvl-greeter.conf.d/lang/tr.qm
chmod 644 release/usr/share/lightdm/lightdm-hvl-greeter.conf.d/resources/bgs/bg1.jpg
chmod 644 release/usr/share/xgreeters/hvl-lightdm-greeter.desktop
chmod 644 release/usr/share/lightdm/lightdm-hvl-greeter.conf.d/hvl-lightdm-greeter.conf.tmp
chmod 755 release/usr/local/bin/hvl-lightdm-greeter

touch release/DEBIAN/md5sums
touch release/DEBIAN/control
find . -type f ! -regex '.*.hg.*' ! -regex '.*?debian-binary.*' ! -regex '.*?DEBIAN.*' -printf '%P ' | xargs md5sum  1>/dev/null 2>/dev/null || true


chmod 775 release/DEBIAN/preinst
chmod 775 release/DEBIAN/postrm
chmod 775 release/DEBIAN/postinst


export VERSION=$(cat VERSION)

export INSTALLSIZE=$(du -s ./release | awk '{print $1}')

echo """Package: lightdm-hvl-greeter
Version: $VERSION
Installed-Size: $INSTALLSIZE
Maintainer: onur kepenek <onurkepnk@gmail.com>
Section: x11
Architecture: amd64
Priority: optional
Depends: libqt5webenginewidgets5, liblightdm-qt5-3-0, libqt5webengine5, libx11-6, libxcursor1, libqt5x11extras5, libqt5positioning5
Description: Enterprise login display manager
 Login screen for enterprise (QT greeter)
""" > release/DEBIAN/control

dpkg-deb -Zgzip --build release
mv release.deb lightdm-hvl-greeter_${VERSION}_amd64.deb
rm -rf release
rm -rf build




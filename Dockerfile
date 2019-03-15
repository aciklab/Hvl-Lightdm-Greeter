FROM ubuntu:18.04

RUN find / -name usersmodel.h

RUN apt-get update \
    && apt-get install -y -qq --no-install-recommends \
        cmake \
	lightdm \
        build-essential \
        qt5-default \
        libqt5webenginewidgets5 \
        qttools5-dev \
        liblightdm-qt5-3-dev \
	libqt5webengine5 \
	libqt5webenginecore5 \
	qtwebengine5-dev \
	libxcursor1 \
	libqt5x11extras5-dev \  
	libxcursor-dev \  
	libxrandr-dev

COPY . /usr/hvl-lightdm-greeter/

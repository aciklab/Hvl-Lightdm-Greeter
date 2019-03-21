# hvl-lightdm-greeter 

[TÜRKÇE](https://github.com/aciklab/Hvl-Lightdm-Greeter/)

## Destination

hvl-lightdm-greeter is a login application for the lightdm displaymanager, written in c++ and qt5. This project inspired by [qt-lightdm-greeter](https://github.com/surlykke/qt-lightdm-greeter) 

This greeter provides to reset expired passwords (Tested with Windows active directory accounts and local accounts). If a password expired. greeter opens password reset page and prompt user to reset password.

Tested on Pardus 17.4 and Xubuntu 18.04

`Login screen`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/loginpage_tr.jpg" width="300" height="300">

`Users screen`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/userspage_en.jpg" width="400" height="300">

`Prompt screen`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/prompt_en.jpg" width="300" height="300">

`Password reset screen`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/reset_en.jpg" width="300" height="300">

## Installing

### Compile

For building, make sure you have; 
- qt5-default
- cmake
- qtwebengine5-dev 
- libqt5x11extras5-dev
- liblightdm-qt5-3-dev
- libxcursor-dev
- libxrandr-dev 
- lightdm
- qttools5-dev-tools

installed on your system

For running, dependencies are : lightdm, libqt5webenginewidgets5, liblightdm-qt5-3-0, libqt5webengine5, libx11-6, libxcursor1, libqt5x11extras5

To install, do:
    
```shell
git clone https://github.com/mertcelen/Hvl-Lightdm-Greeter.git
cd Hvl-Lightdm-Greeter
mkdir build
cd build
cmake ..
make 
sudo make install
```

Update or insert in(to) your `/etc/lightdm/lightdm.conf`, in the `SeatDefaults` section, this line:

    greeter-session=hvl-lightdm-greeter

If this file is not exist create new one.	

### on Pardus Onyedi

```sudo echo "deb [arch=amd64] http://acikdepo.github.io/ onyedi main" > /etc/apt/sources.list.d/acikdepo.list
sudo wget -qO - https://raw.githubusercontent.com/acikdepo/acikdepo.github.io/master/public.key | sudo apt-key add -
sudo apt update
sudo apt install lightdm-hvl-greeter
```
	
## Configuration

The file `/usr/share/lightdm/lightdm-hvl-greeter.conf.d/hvl-lightdm-greeter.conf` allows to configure hvl-lightdm-greeter. For example: 
	-background-image, 
	-positioning of loginform, settingsform and clockform. 
	-System services to wait etc.
The configuration options are documented in that file.

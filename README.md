# Hvl-Lightdm-Greeter

## Installing

Make sure you have cmake, gcc, qt5 and liblightdm-qt5 (and lightdm) installed on your system

Do
    
```shell
git clone https://github.com/surlykke/qt-lightdm-greeter.git
cd qt-lightdm-greeter
mkdir build
cd build
cmake ..
make 
sudo make install
```

## How to enable qt-lightdm greeter

Update or insert in(to) your `/etc/lightdm/lightdm.conf`, in the `SeatDefaults` section, this line:

    greeter-session=qt-lightdm-greeter

## Configure qt-lightdm-greeter

The file `/etc/lightdm/qt-lightdm-greeter.conf` allows for a 
few configurations of qt-lightdm-greeter (background-image, positioning of loginform). 
The configuration options are documented in that file.
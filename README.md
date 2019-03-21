[![Build Status](https://travis-ci.org/onurkepenek/Hvl-Lightdm-Greeter.svg?branch=master)](https://travis-ci.org/onurkepenek/Hvl-Lightdm-Greeter)
![GitHub All Releases](https://img.shields.io/github/downloads/aciklab/Hvl-Lightdm-Greeter/total.svg)
![GitHub release](https://img.shields.io/github/release/aciklab/Hvl-Lightdm-Greeter.svg)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/aciklab/Hvl-Lightdm-Greeter.svg)
![GitHub last commit](https://img.shields.io/github/last-commit/aciklab/Hvl-Lightdm-Greeter.svg)
![GitHub](https://img.shields.io/github/tag/aciklab/Hvl-Lightdm-Greeter.svg)
![GitHub](https://img.shields.io/github/license/aciklab/Hvl-Lightdm-Greeter.svg)

# hvl-lightdm-greeter 

[ENGLISH](https://github.com/aciklab/Hvl-Lightdm-Greeter/blob/master/README_EN.md)

## Tanım

hvl-lightdm-greeter, lightdm yüklü sistemlerde kullanılabilecek bir giriş ekranı uygulamasıdır.  [qt-lightdm-greeter](https://github.com/surlykke/qt-lightdm-greeter) 'dan esinlenilerek c++ ve qt ile yazılmıştır.

Bu giriş ekranı, süresi dolmuş parolaları sıfırlama imkanı verebilmektedir (Windows aktif dizin hesapları ve yerel kullanıcı hesapları ile test edilmiştir). 
Eğer bir parolanın değiştirlmesi gerekiyorsa giriş sırasında kullanıcının parolasını sıfırlaması istenir. Arkasından parola sıfırlama sayfası açılarak kullanıcının parolasını sıfırlaması sağlanır.

Giriş ekranı Pardus 17.4 ve Xubuntu 18.04'de test edilmiştir.


`Giriş ekranı`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/loginpage_tr.jpg" width="300" height="300">

`Kullanıcı seçimi`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/userspage_tr.jpg" width="400" height="300">

`Parola sıfırlama uyarısı`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/prompt_tr.jpg" width="300" height="300">

`Parola sıfırlama Sayfası`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/reset_tr.jpg" width="300" height="300">

## Kurulum

### Derleme

Kodu derlemek için, 
- qt5-default
- cmake
- qtwebengine5-dev 
- libqt5x11extras5-dev
- liblightdm-qt5-3-dev
- libxcursor-dev
- libxrandr-dev 
- lightdm
- qttools5-dev-tools

paketlerinin sisteminizde kurulu olduğuna emin olun

Çalıştırmak için bağımlılıklar şunlardır: lightdm, libqt5webenginewidgets5, liblightdm-qt5-3-0, libqt5webengine5, libx11-6, libxcursor1, libqt5x11extras5

Kurlum için, aşağıdaki komutları çalıştırın:

```shell
git clone https://github.com/mertcelen/Hvl-Lightdm-Greeter.git
cd Hvl-Lightdm-Greeter
mkdir build
cd build
cmake ..
make 
sudo make install
```
 `/etc/lightdm/lightdm.conf`, dosyası içinde, `SeatDefaults` bölümü altına aşağıdaki satırı ekleyin ya da başka bir değere atanmışsa güncelleyin:

    greeter-session=hvl-lightdm-greeter

Eğer böyle bir dosya mevcut değilse oluşturun.	

### Pardus Onyedi üzerine kurulum

```sudo echo "deb [arch=amd64] http://acikdepo.github.io/ onyedi main" > /etc/apt/sources.list.d/acikdepo.list
sudo wget -qO - https://raw.githubusercontent.com/acikdepo/acikdepo.github.io/master/public.key | sudo apt-key add -
sudo apt update
sudo apt install lightdm-hvl-greeter
```

## Yapılandırma

`/usr/share/lightdm/lightdm-hvl-greeter.conf.d/hvl-lightdm-greeter.conf` dosyası içinde kullanılabilecek çeşitli konfigürasyon seçenekleri bulunmaktadır. Örnek olarak: 
	-Arka plan resmi, 
	-Giriş, saat ve ayar formlarının pozisyonları, 
	-Beklenmesi gereken servisler vb..
Bu dosya içinde detaylı açıklamalar yapılmıştır. 

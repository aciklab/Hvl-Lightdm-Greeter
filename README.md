[![Build Status](https://travis-ci.org/onurkepenek/Hvl-Lightdm-Greeter.svg?branch=master)](https://travis-ci.org/onurkepenek/Hvl-Lightdm-Greeter)

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

Kodu derlemek için, cmake, gcc, qt5, lightdm, qtwebengine5-dev, qt5LinguistTools, qt5X11Extras-dev, libxcursor-dev paketlerinin sisteminizde kurulu olduğuna emin olun

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

## hvl-lightdm greeter'ı aktif hale getirmek için;

 `/etc/lightdm/lightdm.conf`, dosyası içinde, `SeatDefaults` bölümü altına aşağıdaki satırı ekleyin ya da başka bir değere atanmışsa güncelleyin:

    greeter-session=hvl-lightdm-greeter

Eğer böyle bir dosya mevcut değilse oluşturun.	
	
## Yapılandırma

`/usr/share/lightdm/lightdm-hvl-greeter.conf.d/hvl-lightdm-greeter.conf` dosyası içinde kullanılabilecek çeşitli konfigürasyon seçenekleri bulunmaktadır. Örnek olarak: 
	-Arka plan resmi, 
	-Giriş, saat ve ayar formlarının pozisyonları, 
	-Beklenmesi gereken servisler vb..
Bu dosya içinde detaylı açıklamalar yapılmıştır. 

[![Build Status](https://api.travis-ci.com/aciklab/Hvl-Lightdm-Greeter.svg?branch=master)](https://travis-ci.com/aciklab/Hvl-Lightdm-Greeter)
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

## Özellikler 

* Oturum açmadan ağ bağlantısı gerçekleşmesi durumunda IP adresi ve istenilen servislerin durumunun gözükebilmesi, istenildiği taktirde logların izlenebilmesi.
* Bir web servis aracılığı ile selfservis parola sıfırlama uygulaması varsa ona qtwebengine ile bağlantı eklenebilmesi.
* Etki alanı veya yerel kullanıcının parola süresi dolduğunda yeni bir popup açılarak düzgün şekilde parola değiştirmeyi sağlamakta.
* PAM'dan dönen promtlar daha doğru şekilde ekrana uyarı göstermekte. (pam_unix ve pam_sssd üzerinde çalışılmıştır, winbind üzerinde cache gibi işlemlerde dönen içeriklerde uyarı metni konusunda eksiklik olabilir)
* Ağ bağlantısı gerçekleşmeden giriş yapılabilecek ekranın gözükmemesi sağlanabilmektedir. (yavaş ağlar için belirli timeout değerine göre)
* İstenilen sayıda kişi ikonu gözükebilmesi. Makine etki alanında ise tüm listeyi çekme yavaşlığının önüne geçilmesi.
* Klasör bazlı arkaplan gösterim modu.
* Maddeler konfigüre edilebilir dosya üzerinde tutulması.
* Dokunmatik ekranlar için Ekran klavyesi seçeneği.

Giriş ekranı Pardus 17.4 ve Xubuntu 18.04'de test edilmiştir.


`Giriş ekranı`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/loginpage_tr.jpg">

`Kullanıcı seçimi`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/userspage_tr.jpg">

`Parola sıfırlama uyarısı`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/prompt_tr.jpg">

`Parola sıfırlama Sayfası`

<img src="https://github.com/aciklab/Hvl-Lightdm-Greeter/raw/master/ss/reset_tr.jpg">

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
- qttools5-dev
- qttools5-dev-tools

paketlerinin sisteminizde kurulu olduğuna emin olun

Çalıştırmak için bağımlılıklar şunlardır: lightdm, libqt5webenginewidgets5, liblightdm-qt5-3-0, libqt5webengine5, libx11-6, libxcursor1, libqt5x11extras5

Kurulum için, aşağıdaki komutları çalıştırın:

```shell
git clone https://github.com/mertcelen/Hvl-Lightdm-Greeter.git
cd Hvl-Lightdm-Greeter
mkdir build && cd build
cmake ..
make 
sudo make install
```
 `/etc/lightdm/lightdm.conf`, dosyası içinde, `SeatDefaults` bölümü altına aşağıdaki satırı ekleyin ya da başka bir değere atanmışsa güncelleyin:

    greeter-session=hvl-lightdm-greeter

Eğer böyle bir dosya mevcut değilse oluşturun.	

### Pardus Onyedi üzerine kurulum

```

sudo echo "deb [arch=amd64] http://acikdepo.github.io/ onyedi main" > /etc/apt/sources.list.d/acikdepo.list
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

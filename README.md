[![Gitter](https://badges.gitter.im/janbar/noson-app.svg)](https://gitter.im/janbar/noson-app?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
![build status](https://github.com/janbar/noson-app/actions/workflows/build-ubuntu-latest.yml/badge.svg?branch=master)
![build status](https://github.com/janbar/noson-app/actions/workflows/build-android-x64.yml/badge.svg?branch=master)

## Noson
A fast and smart SONOS controller for Unix platforms.

Go to [project page](http://janbar.github.io/noson-app/index.html) for further details. Translations are managed by **transifex** at [noson-translations](https://www.transifex.com/janbar/noson/).

<p align="center">
  <img src="http://janbar.github.io/noson-app/download/noson3.png"/>
<p>

## Build instructions

The build can be achieved on any platform supporting Qt version 5.15. See `debian/control` file for more details about dependencies.

### Linux/BSD

Running Noson in "freedesktop" allows the following extra features.
- Streaming the PulseAudio output on your Sonos devices
- MPRIS2 player interface

#### Install the dependencies

###### For Ubuntu(22) (**qtbase5 >= 5.15**)

```bash
$ apt-get install qtbase5-dev qttools5-dev qttools5-dev-tools qtdeclarative5-dev \
qtdeclarative5-dev-tools qtquickcontrols2-5-dev libqt5svg5-dev libqt5svg5 \
qml-module-qt-labs-settings qml-module-qtgraphicaleffects \
qml-module-qtqml-models2 qml-module-qtquick2 qml-module-qtquick-controls2 \
qml-module-qtquick-layouts qml-module-qtquick-particles2 \
qml-module-qtquick-templates2 qml-module-qtquick-window2 \
zlib1g-dev libssl-dev libflac-dev libflac++-dev libpulse-dev \
libdbus-1-dev libqt5dbus5
```
  - Build tools: `build-essential` `git` `cmake >= 3.8.2` `gcc | clang`

###### For Centos / Fedora (**qt5-qtbase >= 5.15**)

```bash
$ yum install qt5-qtbase-devel qt5-qttools-devel qt5-qtdeclarative-devel \
qt5-qtquickcontrols2-devel qt5-qtgraphicaleffects qt5-qtsvg-devel \
qt5-qtsvg zlib-devel openssl-devel flac-devel pulseaudio-libs-devel \
dbus-devel
```
  - Build tools: `git` `cmake >= 3.8.2` `gcc | clang`

  - As needed you have to install **cmake >= 3.8.2**

    ```bash
    $ wget https://github.com/Kitware/CMake/releases/download/v3.14.7/cmake-3.14.7-Linux-x86_64.tar.gz
    $ tar xvfz cmake-3.14.7-Linux-x86_64.tar.gz
    $ export PATH=$(pwd)/cmake-3.14.7-Linux-x86_64/bin:$PATH
    ```

###### For FreeBSD

there is a port/package for FreeBSD available. 
To install the package:

```bash
$ pkg install noson-app
```
To use the port
```bash
$ cd /usr/ports/audio/noson-app && make install clean
```

To build from source

```bash
$ pkg install cmake git bash dbus flac pulseaudio ca_root_nss \
qt5-buildtools qt5-core qt5-dbus qt5-declarative qt5-graphicaleffects \
qt5-gui qt5-network qt5-qmake qt5-quickcontrols2 qt5-svg qt5-widgets \
qt5-xml qt5-xmlpatterns
```

#### Build and install the application

```bash
$ git clone https://github.com/janbar/noson-app.git
$ cd noson-app && mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j5
$ sudo make install
```
  - To uninstall the application type `sudo make uninstall`

### Others supported platforms (without extra features*)
- MacOSX (XCode >= 9.0)
- Android (SDK 21.0, Android >= 16 Lollipop)
- Windows (MSVC 2017)

<small>(\*) PulseAudio is available only with freedesktop (Linux/BSD). So on those platforms you can only stream your local music library.</small>

## Enabling debug output

Launch the *Noson* application from command line as follows.

- `noson-app --debug 2>&1 | tee noson.log`

Also, that will write debug output into the file `noson.log`. **Please be careful to not paste debug log on public area before cleaning it from any credentials**. Debugging registration of music services will log entered credentials.

## Streaming my local music files

Noson will scan music files only from the standard directory containing audio media. Depending on the platform and your language, it could be "~/Music" on freedesktop. When you drop files in this location, you should be able to stream them to your Sonos device with Noson. The local library will be exposed in the page "This Device". Supported files are flac, mp3, mp4 and ogg. An additionnal music location can be configured in the general settings.

## SSDP discovery fails

**You must first disable the running firewall on the system where the app is running, or add extra rules for your firewall profile. The app must listen to Sonos device events to function properly.**

**To add the according to [Sonos](https://support.sonos.com/s/article/688?language=en_US), required rules to `ufw` you can use the following instructions :**.

- Create a file named `noson` in /etc/ufw/applications.d` with the following:

``` ini
[noson]
title=noson Sonos controller app
description=controls Sonos devices on the same network
ports=80,443,445,1400:1410,3400,3401,3405,3445,3500,4070,4444/tcp|136,137,138,139,1900,1901,2869,10243,10280,10281,10282,10283,10284,5353,6969/udp|35382
```

- Update the rules for ufw with:

``` bash
sudo ufw app update noson
sudo ufw allow noson
```

---

In some circumstances your network couldn't allow broadcast/multicast traffic. To workaround the limitation you can configure the application by specifying a Sonos device endpoint.

Launch the *Noson* application from command line as follows.

- `noson-app --deviceurl=http://<ipaddress>:1400` for the GUI (the setting will be persistent).

- `noson-app --cli --deviceurl=http://<ipaddress>:1400` for the CLI tool.

Some SONOS devices could listen on port 3400 instead 1400.

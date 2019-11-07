[![Gitter](https://badges.gitter.im/janbar/noson-app.svg)](https://gitter.im/janbar/noson-app?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Build Status](https://travis-ci.org/janbar/noson-app.svg?branch=master)](https://travis-ci.org/janbar/noson-app)

## Noson
A fast and smart SONOS controller for Linux platforms.

Go to [project page](http://janbar.github.io/noson-app/index.html) for further details. Translations are managed by **transifex** at [noson-translations](https://www.transifex.com/janbar/noson/).

<p align="center">
  <img src="http://janbar.github.io/noson-app/download/noson3.png"/>
<p>

## Build instructions

The build can be achieved on any platform supporting Qt version 5.9. See `debian/control` file for more details about dependencies.

### Linux/BSD

Running Noson in "freedesktop" allows the following extra features.
- Streaming the PulseAudio output on your Sonos devices
- MPRIS2 player interface

#### Install the dependencies

###### For Ubuntu(18) / Debian(10) (**qtbase5 >= 5.9.1**)

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
  - Build tools: `build-essential` `git` `cmake` `g++ >= 4.8.5 | clang >= 3.4`

###### For Centos(7.6) / Fedora(26) (**qt5-qtbase >= 5.9.1**)

```bash
$ yum install qt5-qtbase-devel qt5-qttools-devel qt5-qtdeclarative-devel \
qt5-qtquickcontrols2-devel qt5-qtgraphicaleffects qt5-qtsvg-devel \
qt5-qtsvg zlib-devel openssl-devel flac-devel pulseaudio-libs-devel \
dbus-devel
```
  - Build tools: `git` `cmake >= 3.1.0` `gcc-c++ >= 4.8.5 | clang >= 3.4`

  - As needed you have to install **cmake >= 3.1.0**

    ```bash
    $ wget https://github.com/Kitware/CMake/releases/download/v3.14.7/cmake-3.14.7-Linux-x86_64.tar.gz
    $ tar xvfz cmake-3.14.7-Linux-x86_64.tar.gz
    $ export PATH=$(pwd)/cmake-3.14.7-Linux-x86_64/bin:$PATH
    ```

###### For FreeBSD 12.0

```bash
$ pkg install cmake git bash dbus flac pulseaudio \
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

<small>(\*) PulseAudio is available with freedesktop and obviously you can only stream your local music library.</small>

## Enabling debug output

Launch the *Noson* application from command line as follows.

- `noson-app --debug 2>&1 | tee noson.log`

Also that will write debug output into the file `noson.log`. **Please be carefull to not paste debug log on public area before cleaning it from any credentials**. Debugging registration of music services will log entered credentials.

## SSDP discovery fails

In some circonstances your network couldn't allow broadcast/multicast traffic. To workaround the limitation you can configure the application by specifying the device endpoint (from the release >= 3.16).

Launch the *Noson* application from command line as follows.

- `noson-app --deviceurl=http://<ipaddress>:1400` for the GUI (the setting will be persistent).

- `noson-app --cli --deviceurl=http://<ipaddress>:1400` for the CLI tool.

Some SONOS devices could listen on port 3400 instead 1400.

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

#### Install the dependencies

- For Ubuntu(18) / Debian(10) (**qtbase5 >= 5.9.1**)

  `qtbase5-dev` `qttools5-dev` `qttools5-dev-tools` `qtdeclarative5-dev`
  `qtdeclarative5-dev-tools` `qtquickcontrols2-5-dev` `libqt5svg5-dev` `libqt5svg5`
  `qml-module-qt-labs-settings` `qml-module-qtgraphicaleffects` `qml-module-qtqml-models2`
  `qml-module-qtquick2` `qml-module-qtquick-controls2` `qml-module-qtquick-layouts`
  `qml-module-qtquick-particles2` `qml-module-qtquick-templates2` `qml-module-qtquick-window2`
  `zlib1g-dev` `libssl-dev` `libflac-dev` `libflac++-dev` `libpulse-dev`
  `libdbus-1-dev` `libqt5dbus5`

- For Centos(7.6) (**qt5-qtbase >= 5.9.1**)

  `qt5-qtbase-devel` `qt5-qttools-devel` `qt5-qtdeclarative-devel` `qt5-qtquickcontrols2-devel`
  `qt5-qtsvg-devel` `qt5-qtsvg` `qt5-qtgraphicaleffects` `zlib-devel` `openssl-devel`
  `flac-devel` `pulseaudio-libs-devel` `dbus-devel`

  Install latest CMAKE tools (**cmake >= 3.1.0**)

  - `wget https://github.com/Kitware/CMake/releases/download/v3.14.7/cmake-3.14.7-Linux-x86_64.tar.gz`
  - `tar xvfz cmake-3.14.7-Linux-x86_64.tar.gz`
  - `export PATH=$(pwd)/cmake-3.14.7-Linux-x86_64/bin:$PATH`

#### Build and install the application

  - `git clone https://github.com/janbar/noson-app.git`
  - `cd noson-app && mkdir build && cd build`
  - `cmake -DCMAKE_BUILD_TYPE=Release ..`
  - `make -j5`
  - `sudo make install`


  - To uninstall the application type `sudo make uninstall`

## Enabling debug output

Launch the *Noson* application from command line as follows.

- `noson-app --debug 2>&1 | tee noson.log`

Also that will write debug output into the file `noson.log`. **Please be carefull to not paste debug log on public area before cleaning it from any credentials**. Debugging registration of music services will log entered credentials.

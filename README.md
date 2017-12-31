[![Gitter](https://badges.gitter.im/janbar/noson-app.svg)](https://gitter.im/janbar/noson-app?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

## Noson
A fast and smart SONOS controller for Linux platforms.

Go to [project page](http://janbar.github.io/noson-app/index.html) for further details. Translations are managed by **transifex** at [noson-translations](https://www.transifex.com/janbar/noson/).

<p align="center">
  <img src="http://janbar.github.io/noson-app/download/noson.png"/>
<p>

## Build instructions

The build can be achieved on any platform supporting Qt version 5.9. See `debian/control` file for more details about dependencies.

### Linux/BSD

- `git clone https://github.com/janbar/noson-app.git`
- `cd noson-app && mkdir build && cd build`
- `cmake -DCMAKE_BUILD_TYPE=Release ..`
- `make -j5`
- `sudo make install`

The noson-app files will be placed in `/usr/local/bin` `/usr/local/lib/${ARCH}` `/usr/local/share`.

### MacOS

Install Xcode and CMake tool.
Install required dependencies in `/usr/local` as follows.
- Install a copy of Qt frameworks >= 5.10.0 in `/usr/local/Qt`
- Install a static copy of openSSL >= 1.0.2 in `/usr/local/lib` (`libcrypto.a` , `libssl.a`), and the headers in `/usr/local/include/openssl`.

Setup environment to build as follows.
- `export PATH=/Applications/CMake.app/Contents/bin:$PATH`
- `export QT_DIR=/usr/local/Qt/5.10.0/clang_64`
- `export SOURCE_DIR=$(pwd)/noson-app`
- `export BUILD_DIR=$(pwd)/noson-app/build`

Download the sources and build.
- `git clone https://github.com/janbar/noson-app.git`
- `mkdir $BUILD_DIR && cd $BUILD_DIR`
- `cmake -DCMAKE_PREFIX_PATH=$QT_DIR -DCMAKE_BUILD_TYPE=Release ..`
- `make -j5`

Generate the application bundle `Noson.app`
- `$SOURCE_DIR/macosx/bundle.sh icon`
- `$SOURCE_DIR/macosx/bundle.sh bundle`

## Enabling debug output

Launch the *Noson* application from command line as follows.

- `noson-app --debug 2>&1 | tee noson.log`

Also that will write debug output into the file `noson.log`. **Please be carefull to not paste debug log on public area before cleaning it from any credentials**. Debugging registration of music services will log entered credentials.


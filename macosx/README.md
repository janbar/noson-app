## Build instructions for MacOSX

Install Xcode and CMake tool.

Install required dependencies as follows.
- Install a copy of Qt frameworks >= 5.12.4 in `/Users/Shared/Qt`

Setup environment to build as follows.
- `export PATH=/Applications/CMake.app/Contents/bin:$PATH`
- `export QT_DIR=/Users/Shared/Qt/5.12.4/clang_64`
- `export SOURCE_DIR=$(pwd)/noson-app`
- `export BUILD_DIR=$(pwd)/noson-app/build`

Download the sources and build.
- `git clone https://github.com/janbar/noson-app.git $SOURCE_DIR`
- `mkdir $BUILD_DIR && cd $BUILD_DIR`
- `cmake -DCMAKE_PREFIX_PATH=$QT_DIR -DCMAKE_BUILD_TYPE=Release $SOURCE_DIR`
- `make -j5`

Generate the application bundle `Noson.app`
- `$SOURCE_DIR/macosx/bundle.sh icon`
- `$SOURCE_DIR/macosx/bundle.sh bundle`

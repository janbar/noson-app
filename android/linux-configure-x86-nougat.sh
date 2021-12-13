BDIR=build-x86-nougat
rm -rf $BDIR/*
mkdir -p $BDIR
cd $BDIR

export JAVA_HOME=/home/jlb/bin/jdk1.8.0
export ANDROID_SDK=/home/shared/Android/Sdk
export ANDROID_NDK=/home/shared/Android/android-ndk-r18b
export ANDROID_NATIVE_API_LEVEL=24
export ANDROID_SDK_MINVER=24
export ANDROID_SDK_TARGET=26
export QT_DIR=/home/shared/Qt/5.12.12/android_x86

cmake ../.. -DCMAKE_SYSTEM_NAME=Android \
-DCMAKE_PREFIX_PATH=$QT_DIR \
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
-DCMAKE_MAKE_PROGRAM=$ANDROID_NDK/prebuilt/linux-x86_64/bin/make \
-DCMAKE_BUILD_TYPE=Release \
-DANDROID_ABI="x86" \
-DANDROID_STL_PREFIX="llvm-libc++" \
-DANDROID_STL_SHARED_LIBRARIES="c++_shared" \
-DANDROID_SDK_MINVER=$ANDROID_SDK_MINVER \
-DANDROID_SDK_TARGET=$ANDROID_SDK_TARGET \
-DANDROID_NATIVE_API_LEVEL=$ANDROID_NATIVE_API_LEVEL \
-DQT_ANDROID_PLATFORM_LEVEL=$ANDROID_NATIVE_API_LEVEL \
-DQT_ANDROID_TOOL_PREFIX="i686-linux-android" \
-DQT_ANDROID_SDK_ROOT=$ANDROID_SDK \
-DQT_ANDROID_NDK_ROOT=$ANDROID_NDK \
-DQT_ANDROID_QT_ROOT=$QT_DIR \
-DQT_ANDROID_SDK_BUILDTOOLS_REVISION="21.0.0" \
-DQt5_DIR=$QT_DIR/lib/cmake/Qt5 \
-DQt5Core_DIR=$QT_DIR/lib/cmake/Qt5Core \
-DQt5Gui_DIR=$QT_DIR/lib/cmake/Qt5Gui \
-DQt5Qml_DIR=$QT_DIR/lib/cmake/Qt5Qml \
-DQt5Network_DIR=$QT_DIR/lib/cmake/Qt5Network \
-DQt5Quick_DIR=$QT_DIR/lib/cmake/Qt5Quick \
-DQt5QuickControls2_DIR=$QT_DIR/lib/cmake/Qt5QuickControls2 \
-DQt5Xml_DIR=$QT_DIR/lib/cmake/Qt5Xml \
-DQt5Svg_DIR=$QT_DIR/lib/cmake/Qt5Svg \
-DQt5Widgets_DIR=$QT_DIR/lib/cmake/Qt5Widgets \
$@

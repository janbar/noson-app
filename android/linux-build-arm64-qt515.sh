BUILD_DIR=build-x64
rm -rf $BUILD_DIR/*
mkdir -p $BUILD_DIR

export JAVA_HOME=$HOME/bin/java/jdk-17.0.12
export ANDROID_SDK_ROOT=$HOME/bin/android/sdk
export ANDROID_NDK=$HOME/bin/android/sdk/ndk/22.1.7171670
export QT_DIR=$HOME/bin/Qt/5.15.15/android

cmake .. -B $BUILD_DIR -DCMAKE_SYSTEM_NAME=Android \
-DCMAKE_PREFIX_PATH=$QT_DIR \
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
-DCMAKE_MAKE_PROGRAM=$ANDROID_NDK/prebuilt/linux-x86_64/bin/make \
-DCMAKE_BUILD_TYPE=Release \
-DANDROID_ABI="arm64-v8a" \
-DANDROID_SDK_MINVER=24 \
-DANDROID_SDK_TARGET=26 \
-DANDROID_NATIVE_API_LEVEL=24 \
-DANDROID_SDK_BUILD_TOOLS_REVISION="31.0.0" \
-DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
-DANDROID_NDK=$ANDROID_NDK \
-DQT_ANDROID_PLATFORM_LEVEL=31 \
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
-DQt5QmlModels_DIR=$QT_DIR/lib/cmake/Qt5QmlModels \
-DQt5AndroidExtras_DIR=$QT_DIR/lib/cmake/Qt5AndroidExtras \
$@

[ $? -eq 0 ] && cmake --build $BUILD_DIR --parallel 8

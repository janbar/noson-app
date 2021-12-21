
### Requirements:

- Android SDK 26-28
- Java jdk 1.8
- Qt Creator 4.12

- Qt <= 5.12 : Android NDK r18b
- Qt >= 5.15 : Android NDK r21e

### Configure the build:

The template of the manifest must be created depending of QT/NDK versions.

- Qt 5.12 -> ndk18 -> AndroidManifest.xml.in.ndk18
- Qt 5.15 -> ndk21 -> AndroidManifest.xml.in.ndk21

cp AndroidManifest.xml.in.${NDK_REVISION} AndroidManifest.xml.in

source ./linux-build.sh -DKEYSTORE_FILE=~/.android/janbar.keystore -DKEYSTORE_ALIAS=key0 -DKEYSTORE_PASSWORD=${PASSWORD}

### Extras

- Setup for lollipop (sdk21):
  ANDROID_NATIVE_API_LEVEL=21, ANDROID_SDK_MINVER=21, ANDROID_SDK_TARGET=21
- Setup for marshmallow (sdk23):
  ANDROID_NATIVE_API_LEVEL=23, ANDROID_SDK_MINVER=23, ANDROID_SDK_TARGET=23


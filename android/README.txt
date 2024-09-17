
### Requirements:

- Android SDK 31
- Java jdk 17.0.12
- Qt Creator 4.12

- Qt >= 5.15 : Android NDK r21e

### Configure the build:

source ./linux-build.sh -DKEYSTORE_FILE=~/.android/janbar.keystore -DKEYSTORE_ALIAS=key0 -DKEYSTORE_PASSWORD=${PASSWORD}

### Extras

- Setup for lollipop (sdk21):
  ANDROID_NATIVE_API_LEVEL=21, ANDROID_SDK_MINVER=21, ANDROID_SDK_TARGET=21
- Setup for marshmallow (sdk23):
  ANDROID_NATIVE_API_LEVEL=23, ANDROID_SDK_MINVER=23, ANDROID_SDK_TARGET=23


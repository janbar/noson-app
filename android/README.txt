
Requirements:

- Android NDK r16-r17
- Android SDK 26-28
- Java jdk 1.8
- Qt Creator 4.6-4.10

Configure the build:

source ./configure-${ARCH}.sh -DKEYSTORE_FILE=~/.android/janbar.keystore -DKEYSTORE_ALIAS=key0 -DKEYSTORE_PASSWORD=${PASSWORD}


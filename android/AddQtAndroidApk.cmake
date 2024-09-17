cmake_minimum_required(VERSION 3.8.2)

# Provides:
#  macro add_qt_android_apk
#
# Requires:
#  env ANDROID_SDK_ROOT             The SDK root path
#  env ANDROID_NDK                  The NDK root path
#  env JAVA_HOME                    Path Java JRE supported by SDK
#
#  ANDROID_ABI                      "arm64-v8a"
#  ANDROID_SDK_MINVER               "24"
#  ANDROID_SDK_TARGET               "26"
#  ANDROID_NATIVE_API_LEVEL         Qt5.15 >=23
#  ANDROID_SDK_BUILD_TOOLS_REVISION "29.0.2"
#  QT_ANDROID_PLATFORM_LEVEL        SDK platform (i.e 29)

# store the current source directory for future use
set(QT_ANDROID_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

# check the JAVA_HOME environment variable
# (I couldn't find a way to set it from this script, it has to be defined outside)
set(JAVA_HOME $ENV{JAVA_HOME})
if(NOT JAVA_HOME)
    message(FATAL_ERROR "The JAVA_HOME environment variable is not set. Please set it to the root directory of the JDK.")
endif()

# make sure that the Android toolchain is used
if(NOT ANDROID)
    message(FATAL_ERROR "Trying to use the CMake Android package without the Android toolchain. Please use the provided toolchain (toolchain/android.toolchain.cmake)")
endif()

# find the Qt directory
if(NOT Qt5Core_DIR)
    find_package(Qt5Core REQUIRED)
endif()
get_filename_component(QT_ANDROID_QT_DIR "${Qt5Core_DIR}/../../.." ABSOLUTE)
message(STATUS "Found Qt for Android: ${QT_ANDROID_QT_DIR}")

# find the Android SDK
if(NOT QT_ANDROID_SDK_ROOT)
    set(QT_ANDROID_SDK_ROOT ${ANDROID_SDK_ROOT})
    if(NOT QT_ANDROID_SDK_ROOT)
        set(QT_ANDROID_SDK_ROOT $ENV{ANDROID_SDK_ROOT})
        if(NOT QT_ANDROID_SDK_ROOT)
            message(FATAL_ERROR "Could not find the Android SDK. Please set either the ANDROID_SDK environment or CMake variable to the root directory of the Android SDK")
        endif()
    endif()
endif()
string(REPLACE "\\" "/" QT_ANDROID_SDK_ROOT ${QT_ANDROID_SDK_ROOT}) # androiddeployqt doesn't like backslashes in paths
message(STATUS "Found Android SDK: ${QT_ANDROID_SDK_ROOT}")

# find the Android NDK
if(NOT QT_ANDROID_NDK_ROOT)
    set(QT_ANDROID_NDK_ROOT ${ANDROID_NDK})
    if(NOT QT_ANDROID_NDK_ROOT)
        set(QT_ANDROID_NDK_ROOT $ENV{ANDROID_NDK})
        if(NOT QT_ANDROID_NDK_ROOT)
            message(FATAL_ERROR "Could not find the Android NDK. Please set either the ANDROID_NDK environment or CMake variable to the root directory of the Android NDK")
        endif()
    endif()
endif()
string(REPLACE "\\" "/" QT_ANDROID_NDK_ROOT ${QT_ANDROID_NDK_ROOT}) # androiddeployqt doesn't like backslashes in paths
message(STATUS "Found Android NDK: ${QT_ANDROID_NDK_ROOT}")

# find the Android architecture
if(NOT QT_ANDROID_ARCHITECTURE)
    if("${ANDROID_ABI}" STREQUAL "arm64-v8a")
        set(QT_ANDROID_ARCHITECTURE "aarch64-linux-android")
    elseif("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
        set(QT_ANDROID_ARCHITECTURE "arm-linux-androideabi")
    elseif("${ANDROID_ABI}" STREQUAL "x86_64")
        set(QT_ANDROID_ARCHITECTURE "x86_64-linux-android")
    elseif("${ANDROID_ABI}" STREQUAL "x86")
        set(QT_ANDROID_ARCHITECTURE "i686-linux-android")
    endif()
    if(NOT QT_ANDROID_ARCHITECTURE)
        message(FATAL_ERROR "Please set the QT_ANDROID_ARCHITECTURE CMake variable")
    endif()
endif()

include(CMakeParseArguments)

# define a macro to create an Android APK target
#
# example:
# add_qt_android_apk(my_app_apk my_app
#     NAME "My App"
#     VERSION_CODE 12
#     PACKAGE_NAME "org.mycompany.myapp"
#     PACKAGE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/package-sources
#     KEYSTORE ${CMAKE_CURRENT_LIST_DIR}/mykey.keystore myalias
#     KEYSTORE_PASSWORD xxxx
#     DEPENDS a_linked_target "path/to/a_linked_library.so" ...
#     PLUGINS "path/to/lib_plugin.so" ...
#     ASSETS "path/to/resources" ...
#     INSTALL
#)
#
# the following bind variables could be used to configure the build from files
# AndroidManifest.xml.in, qtdeploy.json.in, build.gradle.in
#  QT_ANDROID_QT_DIR                    Root path of the native Qt framework
#  QT_ANDROID_SDK_ROOT
#  QT_ANDROID_NDK_ROOT
#  QT_ANDROID_SDK_BUILDTOOLS_REVISION
#  QT_ANDROID_ARCHITECTURE
#  QT_ANDROID_MANIFEST_TEMPLATE
#  QT_ANDROID_STL_PATH
#  QT_ANDROID_PRE_COMMANDS

#  ANDROID_ABI
#  ANDROID_SDK_MINVER
#  ANDROID_SDK_TARGET
#  ANDROID_NATIVE_API_LEVEL
#  ANDROID_NDK_HOST_SYSTEM_NAME

#  QT_ANDROID_APP_TARGET                Application target name
#  QT_ANDROID_APP_PATH                  Full path of binary target
#  QT_ANDROID_APP_NAME                  Application name
#  QT_ANDROID_APP_PACKAGE_NAME          Package name
#  QT_ANDROID_APP_VERSION_CODE          Version level identifier
#  QT_ANDROID_APP_VERSION               Version string
#  QT_ANDROID_APP_PACKAGE_SOURCE_ROOT   Root path of package source
#  QT_ANDROID_APP_EXTRA_LIBS            json string for android-extra-libs
#  QT_ANDROID_APP_EXTRA_PLUGINS         json string for android-extra-plugins
#
macro(add_qt_android_apk TARGET SOURCE_TARGET)

    # parse the macro arguments
    cmake_parse_arguments(ARG "INSTALL" "NAME;VERSION_CODE;PACKAGE_NAME;PACKAGE_SOURCES;KEYSTORE_PASSWORD" "DEPENDS;PLUGINS;ASSETS;KEYSTORE" ${ARGN})

    # target name
    set(QT_ANDROID_APP_TARGET "$<TARGET_NAME:${SOURCE_TARGET}>")

    # full file path to the app's main shared library
    set(QT_ANDROID_APP_PATH "$<TARGET_FILE:${SOURCE_TARGET}>")

    # define the application name
    if(ARG_NAME)
        set(QT_ANDROID_APP_NAME ${ARG_NAME})
    else()
        set(QT_ANDROID_APP_NAME ${SOURCE_TARGET})
    endif()

    # define the application package name
    if(ARG_PACKAGE_NAME)
        set(QT_ANDROID_APP_PACKAGE_NAME ${ARG_PACKAGE_NAME})
    else()
        set(QT_ANDROID_APP_PACKAGE_NAME org.qtproject.${SOURCE_TARGET})
    endif()

    # set the Android SDK build-tools revision
    if(ANDROID_SDK_BUILD_TOOLS_REVISION)
        set(QT_ANDROID_SDK_BUILDTOOLS_REVISION ${ANDROID_SDK_BUILD_TOOLS_REVISION})
    else()
        set(ANDROID_SDK_BUILD_TOOLS_REVISION $ENV{ANDROID_SDK_BUILD_TOOLS_REVISION})
        set(QT_ANDROID_SDK_BUILDTOOLS_REVISION ${ANDROID_SDK_BUILD_TOOLS_REVISION})
    endif()

    # get version code from arguments, or generate a fixed one if not provided
    set(QT_ANDROID_APP_VERSION_CODE ${ARG_VERSION_CODE})
    if(NOT QT_ANDROID_APP_VERSION_CODE)
        set(QT_ANDROID_APP_VERSION_CODE 1)
    endif()

    # try to extract the app version from the target properties, or use the version code if not provided
    get_property(QT_ANDROID_APP_VERSION TARGET ${SOURCE_TARGET} PROPERTY VERSION)
    if(NOT QT_ANDROID_APP_VERSION)
        set(QT_ANDROID_APP_VERSION ${QT_ANDROID_APP_VERSION_CODE})
    endif()

    if(NOT ANDROID_SDK_MINVER)
        set(ANDROID_SDK_MINVER ${ANDROID_NATIVE_API_LEVEL})
    endif()
    if(NOT ANDROID_SDK_TARGET)
        set(ANDROID_SDK_TARGET ${ANDROID_SDK_MINVER})
    endif()

    # check if the user provides a custom source package and its own manifest file
    if(ARG_PACKAGE_SOURCES)
        if(EXISTS "${ARG_PACKAGE_SOURCES}/AndroidManifest.xml")
            # custom manifest provided, use the provided source package directly
            set(QT_ANDROID_APP_PACKAGE_SOURCE_ROOT "${ARG_PACKAGE_SOURCES}")
        endif()
    endif()

    # generate a source package directory if we need to configure a manifest file
    if(NOT QT_ANDROID_APP_PACKAGE_SOURCE_ROOT)
        # create our own configured package directory in build dir
        set(QT_ANDROID_APP_PACKAGE_SOURCE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/package")

        # create the manifest from the template file
        set(QT_ANDROID_MANIFEST_TEMPLATE "${QT_ANDROID_SOURCE_DIR}/AndroidManifest.xml.in")
        configure_file(${QT_ANDROID_MANIFEST_TEMPLATE} ${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml @ONLY)

        # define commands that will be added before the APK target build commands, to refresh the source package directory
        set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS}
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}") # clean the destination directory
        set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E make_directory "${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}") # re-create it
        # deploy Qt translations
        set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E make_directory "${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}/assets/translations")
        set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E copy_directory "${QT_ANDROID_QT_DIR}/translations" "${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}/assets/translations")
        # deploy assets
        if(ARG_PACKAGE_SOURCES)
            set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E copy_directory ${ARG_PACKAGE_SOURCES} ${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}) # copy the user package
        endif()
        if(ARG_ASSETS)
            foreach(ASSET_PATH ${ARG_ASSETS})
                get_filename_component(ASSET_NAME "${ASSET_PATH}" NAME)
                set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E make_directory ${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}/assets/${ASSET_NAME})
                set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSET_PATH} ${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}/assets/${ASSET_NAME})
            endforeach()
        endif()
        set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml ${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}/AndroidManifest.xml) # copy the generated manifest
    endif()

    # define the STL library path
    set(QT_ANDROID_STL_PATH "${QT_ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/${ANDROID_NDK_HOST_SYSTEM_NAME}/sysroot/usr/lib/")

    # set the list of dependant libraries (android-extra-libs)
    if(ARG_DEPENDS)
        foreach(LIB ${ARG_DEPENDS})
            if(TARGET ${LIB})
                # item is a CMake target, extract the library path
                set(LIB "$<TARGET_FILE:${LIB}>")
            endif()
            if(EXTRA_LIBS)
                set(EXTRA_LIBS "${EXTRA_LIBS},${LIB}")
            else()
                set(EXTRA_LIBS "${LIB}")
            endif()
        endforeach()
        set(QT_ANDROID_APP_EXTRA_LIBS "\"android-extra-libs\": \"${EXTRA_LIBS}\",")
    endif()

    # set the list of dependant plugins (android-extra-plugins)
    if(ARG_PLUGINS)
        foreach(PLUGIN ${ARG_PLUGINS})
            if(EXTRA_PLUGINS)
                set(EXTRA_PLUGINS "${EXTRA_PLUGINS},${PLUGIN}")
            else()
                set(EXTRA_PLUGINS "${PLUGIN}")
            endif()
        endforeach()
        set(QT_ANDROID_APP_EXTRA_PLUGINS "\"android-extra-plugins\": \"${EXTRA_PLUGINS}\",")
    endif()

    # make sure that the output directory for the Android package exists
    set(QT_ANDROID_APP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_TARGET}-${ANDROID_ABI})
    file(MAKE_DIRECTORY ${QT_ANDROID_APP_BINARY_DIR}/libs/${ANDROID_ABI})

    # create the configuration file that will feed androiddeployqt
    # 1. replace placeholder variables at generation time
    configure_file(${QT_ANDROID_SOURCE_DIR}/qtdeploy.json.in ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json.in @ONLY)
    # 2. evaluate generator expressions at build time
    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json.in
    )
    # 3. Configure build.gradle to properly work with Android Studio import
    if(EXISTS "${QT_ANDROID_SOURCE_DIR}/build.gradle.in")
        configure_file(${QT_ANDROID_SOURCE_DIR}/build.gradle.in ${QT_ANDROID_APP_BINARY_DIR}/build.gradle @ONLY)
    endif()

    # check if the apk must be signed
    if(ARG_KEYSTORE)
        # external timestamp: --tsa http://timestamp.digicert.com)
        set(SIGN_OPTIONS --release --sign ${ARG_KEYSTORE})
        if(ARG_KEYSTORE_PASSWORD)
            set(SIGN_OPTIONS ${SIGN_OPTIONS} --storepass ${ARG_KEYSTORE_PASSWORD})
        endif()
    endif()

    # check if the apk must be installed to the device
    if(ARG_INSTALL)
        set(INSTALL_OPTIONS --reinstall)
    endif()

    # specify the Android API level
    if(QT_ANDROID_PLATFORM_LEVEL)
        set(TARGET_LEVEL_OPTIONS --android-platform android-${QT_ANDROID_PLATFORM_LEVEL})
    endif()

    # determine the build type to pass to androiddeployqt
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND NOT ARG_KEYSTORE)
        set(QT_ANDROID_BUILD_TYPE --debug)
    elseif()
        set(QT_ANDROID_BUILD_TYPE --release)
    endif()

    # create a custom command that will run the androiddeployqt utility to prepare the Android package
    add_custom_target(
        ${TARGET}
        ALL
        DEPENDS ${SOURCE_TARGET}
        ${QT_ANDROID_PRE_COMMANDS}
        # it seems that recompiled libraries are not copied if we don't remove them first
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${QT_ANDROID_APP_BINARY_DIR}/libs/${ANDROID_ABI}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${QT_ANDROID_APP_BINARY_DIR}/libs/${ANDROID_ABI}
        COMMAND ${CMAKE_COMMAND} -E copy ${QT_ANDROID_APP_PATH} ${QT_ANDROID_APP_BINARY_DIR}/libs/${ANDROID_ABI}
        COMMAND ${QT_ANDROID_QT_DIR}/bin/androiddeployqt
        --output ${QT_ANDROID_APP_BINARY_DIR}
        --input ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json
        --gradle
        ${QT_ANDROID_BUILD_TYPE}
        ${TARGET_LEVEL_OPTIONS}
        ${INSTALL_OPTIONS}
        ${SIGN_OPTIONS}
    )
    # Extra for debugging APK: set android:debuggable="true" in AndroidManifest.xml
    # androiddeployqt --gdbserver --no-strip --verbose

endmacro()

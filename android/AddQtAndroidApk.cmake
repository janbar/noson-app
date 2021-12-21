cmake_minimum_required(VERSION 3.0)

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

# find the Qt root directory
if(NOT Qt5Core_DIR)
    find_package(Qt5Core REQUIRED)
endif()
get_filename_component(QT_ANDROID_QT_ROOT "${Qt5Core_DIR}/../../.." ABSOLUTE)
message(STATUS "Found Qt for Android: ${QT_ANDROID_QT_ROOT}")

# find the Android SDK
if(NOT QT_ANDROID_SDK_ROOT)
    set(QT_ANDROID_SDK_ROOT $ENV{ANDROID_SDK})
    if(NOT QT_ANDROID_SDK_ROOT)
        message(FATAL_ERROR "Could not find the Android SDK. Please set either the ANDROID_SDK environment variable, or the QT_ANDROID_SDK_ROOT CMake variable to the root directory of the Android SDK")
    endif()
endif()
string(REPLACE "\\" "/" QT_ANDROID_SDK_ROOT ${QT_ANDROID_SDK_ROOT}) # androiddeployqt doesn't like backslashes in paths
message(STATUS "Found Android SDK: ${QT_ANDROID_SDK_ROOT}")

# find the Android NDK
if(NOT QT_ANDROID_NDK_ROOT)
    set(QT_ANDROID_NDK_ROOT $ENV{ANDROID_NDK})
    if(NOT QT_ANDROID_NDK_ROOT)
        set(QT_ANDROID_NDK_ROOT ${ANDROID_NDK})
        if(NOT QT_ANDROID_NDK_ROOT)
        message(FATAL_ERROR "Could not find the Android NDK. Please set either the ANDROID_NDK environment or CMake variable, or the QT_ANDROID_NDK_ROOT CMake variable to the root directory of the Android NDK")
        endif()
    endif()
endif()
string(REPLACE "\\" "/" QT_ANDROID_NDK_ROOT ${QT_ANDROID_NDK_ROOT}) # androiddeployqt doesn't like backslashes in paths
message(STATUS "Found Android NDK: ${QT_ANDROID_NDK_ROOT}")

include(CMakeParseArguments)

# define a macro to create an Android APK target
#
# example:
# add_qt_android_apk(my_app_apk my_app
#     NAME "My App"
#     VERSION_CODE 12
#     PACKAGE_NAME "org.mycompany.myapp"
#     PACKAGE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/my-android-sources
#     BUILDTOOLS_REVISION "23.0.3"
#     KEYSTORE ${CMAKE_CURRENT_LIST_DIR}/mykey.keystore myalias
#     KEYSTORE_PASSWORD xxxx
#     DEPENDS a_linked_target "path/to/a_linked_library.so" ...
#     PLUGINS "path/to/plugin" ...
#     INSTALL
#)
# 
macro(add_qt_android_apk TARGET SOURCE_TARGET)

    # parse the macro arguments
    cmake_parse_arguments(ARG "INSTALL" "NAME;VERSION_CODE;PACKAGE_NAME;PACKAGE_SOURCES;KEYSTORE_PASSWORD;BUILDTOOLS_REVISION" "DEPENDS;PLUGINS;KEYSTORE" ${ARGN})

    # full file path to the app's main shared library
    set(QT_ANDROID_APP_PATH "$<TARGET_FILE:${SOURCE_TARGET}>")
    if(${Qt5Core_VERSION} VERSION_GREATER_EQUAL 5.14)
        set(QT_ANDROID_SUPPORT_MULTI_ABI ON)
    endif()

    if(QT_ANDROID_SUPPORT_MULTI_ABI)
        # qtandroideploy will append by itself the ANDROID_ABI to the target name
        set(QT_ANDROID_APPLICATION_BINARY "${SOURCE_TARGET}")
    else()
        set(QT_ANDROID_APPLICATION_BINARY ${QT_ANDROID_APP_PATH})
    endif()

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
    if(ARG_BUILDTOOLS_REVISION)
        set(QT_ANDROID_SDK_BUILDTOOLS_REVISION ${ARG_BUILDTOOLS_REVISION})
    else()
        set(QT_ANDROID_SDK_BUILDTOOLS_REVISION "")
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
            set(QT_ANDROID_APP_PACKAGE_SOURCE_ROOT ${ARG_PACKAGE_SOURCES})
        elseif(EXISTS "${ARG_PACKAGE_SOURCES}/AndroidManifest.xml.in")
            # custom manifest template provided
            set(QT_ANDROID_MANIFEST_TEMPLATE "${ARG_PACKAGE_SOURCES}/AndroidManifest.xml.in")
        endif()
    endif()

    # generate a source package directory if none was provided, or if we need to configure a manifest file
    if(NOT QT_ANDROID_APP_PACKAGE_SOURCE_ROOT)
        # create our own configured package directory in build dir
        set(QT_ANDROID_APP_PACKAGE_SOURCE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/package")

        # create the manifest from the template file
        if(NOT QT_ANDROID_MANIFEST_TEMPLATE)
            set(QT_ANDROID_MANIFEST_TEMPLATE "${QT_ANDROID_SOURCE_DIR}/AndroidManifest.xml.in")
        endif()
        configure_file(${QT_ANDROID_MANIFEST_TEMPLATE} ${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml @ONLY)

        # define commands that will be added before the APK target build commands, to refresh the source package directory
        set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS}
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}) # clean the destination directory
        set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E make_directory ${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}) # re-create it
        if(ARG_PACKAGE_SOURCES)
            set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E copy_directory ${ARG_PACKAGE_SOURCES} ${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}) # copy the user package
        endif()
        set(QT_ANDROID_PRE_COMMANDS ${QT_ANDROID_PRE_COMMANDS} COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/AndroidManifest.xml ${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}/AndroidManifest.xml) # copy the generated manifest
    endif()

    # newer NDK toolchains don't define ANDROID_STL_PREFIX anymore,
    # so this is a fallback to the only supported value in recent versions
    if(NOT ANDROID_STL_PREFIX)
        if(ANDROID_STL MATCHES "^c\\+\\+_")
            set(ANDROID_STL_PREFIX llvm-libc++)
        endif()
    endif()
    if(NOT ANDROID_STL_PREFIX)
        message(WARNING "Failed to determine ANDROID_STL_PREFIX value for ANDROID_STL=${ANDROID_STL}")
    endif()

    if(QT_ANDROID_SUPPORT_MULTI_ABI)
        # from Qt 5.14 qtandroideploy will find the correct stl.
        set(QT_ANDROID_STL_PATH "${QT_ANDROID_NDK_ROOT}/sources/cxx-stl/${ANDROID_STL_PREFIX}/libs")
    else()
        # define the STL shared library path
        # up until NDK r18, ANDROID_STL_SHARED_LIBRARIES is populated by the NDK's toolchain file
        # since NDK r19, the only option for a shared STL library is libc++_shared
        if(ANDROID_STL_SHARED_LIBRARIES)
            list(GET ANDROID_STL_SHARED_LIBRARIES 0 STL_LIBRARY_NAME) # we can only give one to androiddeployqt
            if(ANDROID_STL_PATH)
                set(QT_ANDROID_STL_PATH "${ANDROID_STL_PATH}/libs/${ANDROID_ABI}/lib${STL_LIBRARY_NAME}.so")
            else()
                set(QT_ANDROID_STL_PATH "${QT_ANDROID_NDK_ROOT}/sources/cxx-stl/${ANDROID_STL_PREFIX}/libs/${ANDROID_ABI}/lib${STL_LIBRARY_NAME}.so")
            endif()
        elseif(ANDROID_STL STREQUAL c++_shared)
            set(QT_ANDROID_STL_PATH "${QT_ANDROID_NDK_ROOT}/sources/cxx-stl/${ANDROID_STL_PREFIX}/libs/${ANDROID_ABI}/libc++_shared.so")
        else()
            message(WARNING "ANDROID_STL (${ANDROID_STL}) isn't a known shared stl library."
                "You should consider setting ANDROID_STL to c++_shared (like Qt).")
            set(QT_ANDROID_STL_PATH "${QT_ANDROID_NDK_ROOT}/sources/cxx-stl/${ANDROID_STL_PREFIX}/libs/${ANDROID_ABI}/libc++_shared.so")
        endif()
    endif()

    # From Qt 5.14 qtandroideploy "target-architecture" is no longer valid in input file
    # It have been replaced by "architectures": { "${ANDROID_ABI}": "${ANDROID_ABI}" }
    # This allow to package multiple ABI in a single apk
    # For now we only support single ABI build with this script (to ensure it work with Qt5.14 & Qt5.15)
    if(QT_ANDROID_SUPPORT_MULTI_ABI)
        set(QT_ANDROID_ARCHITECTURES "\"${ANDROID_ABI}\":\"${ANDROID_ABI}\"")
    endif()

    # set the list of dependant libraries
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

    # deploy Qt translations
    set(EXTRA_PLUGINS "${QT_ANDROID_QT_ROOT}/translations")

    # set the list of dependant plugins
    if(ARG_PLUGINS)
        foreach(PLUGIN ${ARG_PLUGINS})
            if(EXTRA_PLUGINS)
                set(EXTRA_PLUGINS "${EXTRA_PLUGINS},${PLUGIN}")
            else()
                set(EXTRA_PLUGINS "${PLUGIN}")
            endif()
        endforeach()
    endif()
    set(QT_ANDROID_APP_EXTRA_PLUGINS "\"android-extra-plugins\": \"${EXTRA_PLUGINS}\",")

    # determine whether to use the gcc- or llvm/clang- toolchain;
    # if ANDROID_USE_LLVM was explicitly set, use its value directly,
    # otherwise ANDROID_TOOLCHAIN value (set by the NDK's toolchain file)
    # says whether llvm/clang or gcc is used
    if(DEFINED ANDROID_USE_LLVM)
        string(TOLOWER "${ANDROID_USE_LLVM}" QT_ANDROID_USE_LLVM)
    elseif(ANDROID_TOOLCHAIN STREQUAL clang)
        set(QT_ANDROID_USE_LLVM "true")
    else()
        set(QT_ANDROID_USE_LLVM "false")
    endif()

    # set some toolchain variables used by androiddeployqt;
    # unfortunately, Qt tries to build paths from these variables although these full paths
    # are already available in the toochain file, so we have to parse them
    string(REGEX MATCH "${ANDROID_NDK}/toolchains/llvm/prebuilt/.*" ANDROID_USE_LLVM_PARSED ${ANDROID_TOOLCHAIN_ROOT})
    if(ANDROID_USE_LLVM_PARSED)
        set(QT_ANDROID_USE_LLVM "true")
        set(QT_ANDROID_TOOLCHAIN_PREFIX "llvm")
        set(QT_ANDROID_TOOLCHAIN_VERSION "")
    else()
        set(QT_ANDROID_USE_LLVM "false")
        string(REGEX MATCH "${ANDROID_NDK}/toolchains/(.*)-(.*)/prebuilt/.*" ANDROID_TOOLCHAIN_PARSED ${ANDROID_TOOLCHAIN_ROOT})
        if(ANDROID_TOOLCHAIN_PARSED)
            set(QT_ANDROID_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
            set(QT_ANDROID_TOOLCHAIN_VERSION ${CMAKE_MATCH_2})
        else()
            message(FATAL_ERROR "Failed to parse ANDROID_TOOLCHAIN_ROOT to get toolchain prefix and version")
        endif()
    endif()

    # make sure that the output directory for the Android package exists
    set(QT_ANDROID_APP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_TARGET}-${ANDROID_ABI})
    file(MAKE_DIRECTORY ${QT_ANDROID_APP_BINARY_DIR}/libs/${ANDROID_ABI})

    # create the configuration file that will feed androiddeployqt
    configure_file(${QT_ANDROID_SOURCE_DIR}/qtdeploy.json.in ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json @ONLY)

    # create the configuration file that will feed androiddeployqt
    # 1. replace placeholder variables at generation time
    configure_file(${QT_ANDROID_SOURCE_DIR}/qtdeploy.json.in ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json.in @ONLY)
    # 2. evaluate generator expressions at build time
    file(GENERATE
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json
        INPUT ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json.in
    )

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
        COMMAND ${QT_ANDROID_QT_ROOT}/bin/androiddeployqt
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

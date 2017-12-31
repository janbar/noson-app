#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source $DIR/env-path.sh

APP_VERSION=0
BUILD_TIME=$(date +"%y-%m-%dT%H:%M:%S")
BUILD_HASH_KEY=0


function buildIcon {
    rm -rf $BUILD_BIN_DIR/$APP_NAME.iconset
    mkdir $BUILD_BIN_DIR/$APP_NAME.iconset
    sips -z 16 16     $SRC_RESOURCES_DIR/$APP_NAME.png --out $BUILD_BIN_DIR/$APP_NAME.iconset/icon_16x16.png
    sips -z 32 32     $SRC_RESOURCES_DIR/$APP_NAME.png --out $BUILD_BIN_DIR/$APP_NAME.iconset/icon_16x16@2x.png
    sips -z 32 32     $SRC_RESOURCES_DIR/$APP_NAME.png --out $BUILD_BIN_DIR/$APP_NAME.iconset/icon_32x32.png
    sips -z 64 64     $SRC_RESOURCES_DIR/$APP_NAME.png --out $BUILD_BIN_DIR/$APP_NAME.iconset/icon_32x32@2x.png
    sips -z 128 128   $SRC_RESOURCES_DIR/$APP_NAME.png --out $BUILD_BIN_DIR/$APP_NAME.iconset/icon_128x128.png
    sips -z 256 256   $SRC_RESOURCES_DIR/$APP_NAME.png --out $BUILD_BIN_DIR/$APP_NAME.iconset/icon_128x128@2x.png
    sips -z 256 256   $SRC_RESOURCES_DIR/$APP_NAME.png --out $BUILD_BIN_DIR/$APP_NAME.iconset/icon_256x256.png
    sips -z 512 512   $SRC_RESOURCES_DIR/$APP_NAME.png --out $BUILD_BIN_DIR/$APP_NAME.iconset/icon_256x256@2x.png
    sips -z 512 512   $SRC_RESOURCES_DIR/$APP_NAME.png --out $BUILD_BIN_DIR/$APP_NAME.iconset/icon_512x512.png
    cp -v            $SRC_RESOURCES_DIR/$APP_NAME.png       $BUILD_BIN_DIR/$APP_NAME.iconset/icon_512x512@2x.png
    iconutil -c icns -o $BUILD_BIN_DIR/$APP_NAME.icns $BUILD_BIN_DIR/$APP_NAME.iconset
    # rm -r $BUILD_BIN_DIR/$APP_NAME.iconset
}


function buildAppStructure {
    # structure bundle
    # Noson.app/
    #   Contents/
    #      Info.plist
    #      MacOS/
    #         noson-gui
    #      Resources/
    #         noson.icns
    #         qml/NosonApp
    #      Frameworks/
    #         <libs>
    #      PlugIns
    #         <libs>
    
    rm -rf $BUILD_BUNDLE_DIR
    mkdir $BUILD_BUNDLE_DIR
    
    # predefined data
    cp -v -R $SRC_RESOURCES_DIR/Contents $BUILD_BUNDLE_DIR
    mkdir -p $BUILD_BUNDLE_RES_DIR

    # new icon, if one has been created (otherwise the one from predefined data
    if [ -f "$BUILD_BIN_DIR/$APP_NAME.icns" ]; then
        cp -v $BUILD_BIN_DIR/$APP_NAME.icns $BUILD_BUNDLE_RES_DIR/
    fi
    
    # binary
    mkdir -p $BUILD_BUNDLE_APP_DIR
    cp -v $BUILD_DIR/gui/noson-gui  $BUILD_BUNDLE_APP_FILE

    mkdir -p $BUILD_BUNDLE_RES_DIR/qml
    cp -v -r $BUILD_DIR/backend/NosonApp $BUILD_BUNDLE_RES_DIR/qml/
    
    mkdir -p $BUILD_BUNDLE_RES_QM_DIR
    mkdir -p $BUILD_BUNDLE_RES_BIN_DIR
}

function qtDeploy {
    # -no-strip 
    $QT_DIR/bin/macdeployqt $BUILD_BUNDLE_DIR -always-overwrite -verbose=3 -executable=$BUILD_BUNDLE_APP_FILE
}

function printLinkingApp {
    printLinking $BUILD_BUNDLE_APP_FILE

    for F in `find $BUILD_BUNDLE_FRW_DIR -type f -type f \( -iname "*.dylib" -o -iname "*.so" \)`    
    do
        printLinking $F
    done
    
    for F in `find $BUILD_BUNDLE_FRW_DIR/Qt*.framework/Versions/5 -type f -maxdepth 1` 
    do
        printLinking $F
    done

    for F in `find $BUILD_BUNDLE_PLUGIN_DIR -type f -type f \( -iname "*.dylib" -o -iname "*.so" \)` 
    do
        printLinking $F
    done
    
    checkLibraries $BUILD_BUNDLE_APP_FILE

    for F in `find $BUILD_BUNDLE_FRW_DIR -type f -type f \( -iname "*.dylib" -o -iname "*.so" \)`    
    do
        checkLibraries $F
    done
    
    for F in `find $BUILD_BUNDLE_FRW_DIR/Qt*.framework/Versions/5 -type f -maxdepth 1` 
    do
        checkLibraries $F
    done

    for F in `find $BUILD_BUNDLE_PLUGIN_DIR -type f -type f \( -iname "*.dylib" -o -iname "*.so" \)` 
    do
        checkLibraries $F
    done   
}

function adjustLinking {
 
    for F in `find $BUILD_BUNDLE_PLUGIN_DIR -type f -type f \( -iname "*.dylib" -o -iname "*.so" \)` 
    do 
        adjustLinkQt $F "libq"
    done

    for F in `find $BUILD_BUNDLE_FRW_DIR/Qt*.framework/Versions/5 -type f -maxdepth 1` 
    do 
        adjustLinkQt $F "Qt"
        adjustLinkQt $F "libdbus"
    done

    for F in `find $BUILD_BUNDLE_FRW_DIR -type f -type f \( -iname "*.dylib" -o -iname "*.so" \)` 
    do 
        adjustLinkQt $F "Qt"
        adjustLinkQt $F "libdbus"
    done

    adjustLinkQt $BUILD_BUNDLE_APP_FILE "Qt"
}

function adjustLinkQt {
    F=$1 # file
    L=$2 # search condition
    FREL=${F##*/}

    for P in `otool -L $F | awk '{print $1}'`
    do
        #  replace double slashes
        if [[ "$P" == *//* ]]; then 
            PSLASH=$(echo $P | sed 's,//,/,g')
            sudo install_name_tool -change $P $PSLASH $F
        fi

        LIB=${P##*/}
        LIB=${LIB%%:}
        PREL="@executable_path/../Frameworks/$LIB"

        if [[ "$P" == *".framework"* ]]; then
            LIB_VERSION=Versions/5
            LIB=$LIB.framework/$LIB_VERSION/$LIB
            PREL="@executable_path/../Frameworks/$LIB"
        elif [[ "$P" == *"plugins"* ]]; then
            # subdirectory for PlugIns
            LIB=${P##*plugins/} # remove prepart
            PREL="@executable_path/../PlugIns/$LIB"
        fi

#echo "F    = $F"
#echo "P    = $P"
#echo "LIB  = $LIB"
#echo "FREL = $FREL"
#echo "PREL = $PREL"
#echo "L    = $L"
#echo "-----"
		if [[ "$LIB" == *"$FREL" ]]; then
            echo "name_tool: $FREL >> $PREL ($P)"
            sudo install_name_tool -id $PREL $F
        elif [[ "$P" == *$L* ]]; then
            echo "name_tool: $FREL > $PREL ($P)"
            sudo install_name_tool -change $P $PREL $F
        fi
    done
}


function checkLibraries {
	F=$1 # file
	DIR=${BUILD_BUNDLE_APP_FILE%/*}
	
	for P in `otool -L $F | awk '{print $1}'`
    do
    	if [[ "$P" == "@executable_path"* ]]; then
    		FREL=${P##@executable_path}
    		LIB=${DIR}${FREL}
    		#echo "LIB = $LIB"
    		if [ ! -e $LIB ]; then
    			echo "referenced library not bundled: $P"
    		fi
    	fi
    	if [[ "$P" == "/"* && "$P" != "/System/Library/"* && "$P" != "/usr/lib/"* && "$P" != *":" ]]; then
    		echo "external library: $P"
    	fi
    done
}

function copyAdditionalLibraries {
    #cp -v -R $QT_DIR/lib/QtADDITIONAL.framework $BUILD_BUNDLE_FRW_DIR

    # Additional plugins
    cp -v $QT_DIR/plugins/imageformats/libqsvg.dylib $BUILD_BUNDLE_PLUGIN_DIR/imageformats/

    # QML
    cp -v -R $QT_DIR/qml/Qt $BUILD_BUNDLE_RES_DIR/qml/
    cp -v -R $QT_DIR/qml/QtGraphicalEffects $BUILD_BUNDLE_RES_DIR/qml/
    cp -v -R $QT_DIR/qml/QtQuick $BUILD_BUNDLE_RES_DIR/qml/
    cp -v -R $QT_DIR/qml/QtQuick.2 $BUILD_BUNDLE_RES_DIR/qml/
    cp -v -R $QT_DIR/qml/QtQml $BUILD_BUNDLE_RES_DIR/qml/
}

function copyExternalFiles {
    declare -a languages=("en" "fr" "nl" "de" "it" "es" "cs" "da")
    for lang in "${languages[@]}"; do
        cp -v $QT_DIR/translations/*_${lang}.qm $BUILD_BUNDLE_RES_QM_DIR
    done
}


function adjustLinkingExtTools {
    for F in `find $BUILD_BUNDLE_RES_BIN_DIR -type f ! \( -name "*.py" \)`
    do
        adjustLinkQt $F "/usr/local/lib/"
    done
}


function printLinkingExtTools {
    for F in `find $BUILD_BUNDLE_RES_BIN_DIR -type f ! \( -name "*.py" \)`
    do
        printLinking $F
    done
}


function copyExtTools {
    #cp -v $EXT_DIR/bin/*                       $BUILD_BUNDLE_RES_BIN_DIR
    echo "None."
}


function printLinking {
    echo "--------------------"
    echo "otool $1"
    otool -L $1
    echo "--------------------"
}

function archiveBundle {
    ARCHIVE=$(printf "%s/%s-MacOSX_%s.tar.gz" "$BUILD_DIR" "$APP_NAME" "$APP_VERSION")
    echo $ARCHIVE
    rm $ARCHIVE

    tar -zcvf $ARCHIVE $APP_BUNDLE
}


function extractVersion {    
    APP_VERSION=$(sed -n 's/.*APP_VERSION.*\"\(.*\)\".*/\1/p' $SOURCE_DIR/CMakeLists.txt)
}

function readRevisionHash {
    cd $SOURCE_DIR
    BUILD_HASH_KEY=$(git rev-parse HEAD)
}


function updateInfoPlist {
    /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString $APP_VERSION" "$BUILD_BUNDLE_CONTENTS_DIR/Info.plist"
    /usr/libexec/PlistBuddy -c "Set :CFBundleVersion $APP_VERSION"            "$BUILD_BUNDLE_CONTENTS_DIR/Info.plist"
    /usr/libexec/PlistBuddy -c "Set :BuildHashKey $BUILD_HASH_KEY"            "$BUILD_BUNDLE_CONTENTS_DIR/Info.plist"
    /usr/libexec/PlistBuddy -c "Set :BuildTime $BUILD_TIME"                   "$BUILD_BUNDLE_CONTENTS_DIR/Info.plist"
}


if [[ "$1" == "icon" ]]; then
    buildIcon
fi
if [[ "$1" == "bundle" ]]; then
    echo "---extract version -----------------"
    extractVersion
    readRevisionHash
    echo "---build bundle --------------------"
    buildAppStructure
    echo "---replace version string ----------"
    updateInfoPlist
    echo "---qt deploy tool ------------------"
    qtDeploy
    echo "---copy libraries ------------------"
    copyAdditionalLibraries
    echo "---copy external files -------------"
    copyExternalFiles
    echo "---adjust linking ------------------"
    adjustLinking
    echo "---external tools ------------------"
    copyExtTools
    adjustLinkingExtTools
    printLinkingExtTools
    echo "------------------------------------"
    # chmod a+x $BUILD_BUNDLE_DIR/Contents/Frameworks/*
fi
if [[ "$1" == "info" ]]; then
    printLinkingApp
fi
if [[ "$1" == "archive" ]]; then
    extractVersion
    archiveBundle
fi


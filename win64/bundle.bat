rem Batch file to copy the necessary files for the Windows Installer

set QT_DIR="C:\Qt\5.12.6\msvc2017_64"
rem get the VC redistributable installer from https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
set VCREDIST_DIR="C:\Qt"
rem the path to the source directory
set SOURCE_DIR="..\"
rem the path to the build directory
set BUILD_DIR="..\..\build-noson-app\"

rem Copy Files
set FILES="Files"
rmdir /s/q %FILES%
mkdir %FILES%

rem Copy Qt depends
copy %QT_DIR%\bin\Qt5Core.dll %FILES%
copy %QT_DIR%\bin\Qt5DBus.dll %FILES%
copy %QT_DIR%\bin\Qt5Gui.dll %FILES%
copy %QT_DIR%\bin\Qt5Network.dll %FILES%
copy %QT_DIR%\bin\Qt5OpenGL.dll %FILES%
copy %QT_DIR%\bin\Qt5PrintSupport.dll %FILES%
copy %QT_DIR%\bin\Qt5Qml.dll %FILES%
copy %QT_DIR%\bin\Qt5Quick.dll %FILES%
copy %QT_DIR%\bin\Qt5QuickWidgets.dll %FILES%
copy %QT_DIR%\bin\Qt5QuickControls2.dll %FILES%
copy %QT_DIR%\bin\Qt5QuickTemplates2.dll %FILES%
copy %QT_DIR%\bin\Qt5Svg.dll %FILES%
copy %QT_DIR%\bin\Qt5Widgets.dll %FILES%
copy %QT_DIR%\bin\Qt5Xml.dll %FILES%
copy %QT_DIR%\bin\libEGL.dll %FILES%
copy %QT_DIR%\bin\libGLESv2.dll %FILES%
mkdir %FILES%\imageformats
copy %QT_DIR%\plugins\imageformats\qgif.dll %FILES%\imageformats
copy %QT_DIR%\plugins\imageformats\qicns.dll %FILES%\imageformats
copy %QT_DIR%\plugins\imageformats\qico.dll %FILES%\imageformats
copy %QT_DIR%\plugins\imageformats\qjpeg.dll %FILES%\imageformats
copy %QT_DIR%\plugins\imageformats\qsvg.dll %FILES%\imageformats
copy %QT_DIR%\plugins\imageformats\qtiff.dll %FILES%\imageformats
copy %QT_DIR%\plugins\imageformats\qtga.dll %FILES%\imageformats
mkdir %FILES%\platforms
copy %QT_DIR%\plugins\platforms\qwindows.dll %FILES%\platforms
mkdir %FILES%\printsupport
copy %QT_DIR%\plugins\printsupport\windowsprintersupport.dll %FILES%\printsupport
mkdir %FILES%\iconengines
copy %QT_DIR%\plugins\iconengines\qsvgicon.dll %FILES%\iconengines
mkdir %FILES%\bearer
copy %QT_DIR%\plugins\bearer\qgenericbearer.dll %FILES%\bearer

mkdir %FILES%\Qt
xcopy /E %QT_DIR%\qml\Qt %FILES%\Qt
mkdir %FILES%\QtGraphicalEffects
xcopy /E %QT_DIR%\qml\QtGraphicalEffects %FILES%\QtGraphicalEffects
mkdir %FILES%\QtQml
xcopy /E %QT_DIR%\qml\QtQml %FILES%\QtQml
mkdir %FILES%\QtQuick
xcopy /E %QT_DIR%\qml\QtQuick %FILES%\QtQuick
mkdir %FILES%\QtQuick.2
xcopy /E %QT_DIR%\qml\QtQuick.2 %FILES%\QtQuick.2

rem Copy Qt translations
rem Qt5: see http://doc.qt.io/qt-5/linguist-programmers.html
mkdir %FILES%\translations
copy %QT_DIR%\translations\qtbase_en.qm %FILES%\translations

rem Copy MSVC Redist Files
copy %VCREDIST_DIR%\vc_redist.x64.exe %FILES%

rem Copy OpenSSL depends
copy %BUILD_DIR%\backend\lib\openssl-1.1-build\crypto\"libcrypto-1_1-x64.dll" %FILES%
copy %BUILD_DIR%\backend\lib\openssl-1.1-build\ssl\"libssl-1_1-x64.dll" %FILES%

rem Copy FLAC depends
copy %BUILD_DIR%\backend\lib\flac-build\"libFLAC.dll" %FILES%
copy %BUILD_DIR%\backend\lib\flac-build\"libFLAC++.dll" %FILES%

rem Copy application Files
copy %BUILD_DIR%\gui\noson-gui.exe %FILES%
copy %BUILD_DIR%\backend\cli\noson-cli.exe %FILES%
mkdir %FILES%\NosonApp
copy %BUILD_DIR%\backend\qml\NosonApp\NosonApp.dll %FILES%\NosonApp
copy %BUILD_DIR%\backend\qml\NosonApp\qmldir %FILES%\NosonApp
mkdir %FILES%\NosonMediaScanner
copy %BUILD_DIR%\backend\qml\NosonMediaScanner\NosonMediaScanner.dll %FILES%\NosonMediaScanner
copy %BUILD_DIR%\backend\qml\NosonMediaScanner\qmldir %FILES%\NosonMediaScanner
mkdir %FILES%\NosonThumbnailer
copy %BUILD_DIR%\backend\qml\NosonThumbnailer\NosonThumbnailer.dll %FILES%\NosonThumbnailer
copy %BUILD_DIR%\backend\qml\NosonThumbnailer\qmldir %FILES%\NosonThumbnailer

rem Copy qt.conf
copy %SOURCE_DIR%\win64\qt.conf %FILES%
rem Copy noson.ico
copy %SOURCE_DIR%\win64\noson.ico %FILES%

pause

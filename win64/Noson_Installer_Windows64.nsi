;NSIS Installer Script for https://github.com/janbar/noson-app/win64

;NSIS References/Documentation
;http://nsis.sourceforge.net/Docs/Modern%20UI%202/Readme.html
;http://nsis.sourceforge.net/Docs/Modern%20UI/Readme.html
;http://nsis.sourceforge.net/Docs/Chapter4.html
;http://nsis.sourceforge.net/Many_Icons_Many_shortcuts

;Deployment issues
;Deploying Qt5 for Windows:
;  http://qt-project.org/doc/qt-5/windows-deployment.html
;Deploying MSVC runtime libraries
;  http://msdn.microsoft.com/en-us/library/dd293574.aspx ==> Central Deployment is preferred: by using a redistributable package enables automatic updating by Microsoft.
;  http://msdn.microsoft.com/en-us/library/8kche8ah.aspx ==> Distribute msvcr120.dll and msvcp120.dll
;  http://www.microsoft.com/en-us/download/details.aspx?id=40784 ==> Download the vcredist_x64.exe from here !!!
;  http://msdn.microsoft.com/en-us/vstudio/dn501987.aspx ==> Legal stuff
;  https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads --> MSVC2017

;Revision Log
; 06-Aug-2019 First version of Noson installer for msvc2017_64

;=================== BEGIN SCRIPT ====================
; Include for nice Setup UI, see http://nsis.sourceforge.net/Docs/Modern%20UI%202/Readme.html
!include MUI2.nsh

;------------------------------------------------------------------------
; Modern UI2 definition                                                  -
;------------------------------------------------------------------------
; Description
Name "Noson"

;Default installation folder
InstallDir "$PROGRAMFILES64\Noson"

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\Noson" ""

;Request application privileges for Windows Vista
RequestExecutionLevel admin


; The file to write
OutFile "Noson_Installer_Windows64.exe"

;------------------------------------------------------------------------
; Modern UI definition                                                    -
;------------------------------------------------------------------------
;!define MUI_COMPONENTSPAGE_SMALLDESC ;No value
!define MUI_INSTFILESPAGE_COLORS "FFFFFF 000000" ;Two colors

!define MUI_ICON "noson.ico"
!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_BITMAP "mui_header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "mui_welcome.bmp"

; Page welcome description
!define MUI_WELCOMEPAGE_TITLE "Noson"
!define MUI_WELCOMEPAGE_TITLE_3LINES
!define MUI_WELCOMEPAGE_TEXT "The fast and smart controller for your SONOS devices. \
You can browse your music library and play track or radio on any zones. \
You can manage grouping zones, queue, and playlists, and fully control the playback.$\r$\n$\r$\n\
Site: http://janbar.github.io/noson-app"

!define MUI_LICENSEPAGE_CHECKBOX

;------------------------------------------------------------------------
; Pages definition order                                                -
;------------------------------------------------------------------------
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
Var StartMenuFolder
!insertmacro MUI_PAGE_STARTMENU "Application" $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
;------------------------------------------------------------------------

;------------------------------------------------------------------------
;Uninstaller                                                            -
;------------------------------------------------------------------------
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language settings
!insertmacro MUI_LANGUAGE "English"


;------------------------------------------------------------------------
; Component add                                                            -
;------------------------------------------------------------------------
;Components description

Section "MSVC++ 2017 Runtime" MSVC

  SetOutPath "$INSTDIR"
  File Files\VC_redist.x64.exe
  ExecWait '"$INSTDIR\VC_redist.x64.exe"'
  Delete "$INSTDIR\VC_redist.x64.exe"

SectionEnd
LangString DESC_MSVC ${LANG_ENGLISH} "Microsoft Visual C++ 2017 SP1 Runtime Libraries. Typically already installed on your PC. You only need to install them if it doesn't work without ;-)."

Section "Noson" Noson
  SectionIn RO

  ;Install for all users
  SetShellVarContext all

  ;BEGIN Noson Files
  SetOutPath "$INSTDIR"
    File Files\noson-gui.exe
    File Files\noson-cli.exe
    File Files\noson.ico
  SetOutPath "$INSTDIR\NosonApp"
    File Files\NosonApp\NosonApp.dll
    File Files\NosonApp\qmldir
  SetOutPath "$INSTDIR\NosonMediaScanner"
    File Files\NosonMediaScanner\NosonMediaScanner.dll
    File Files\NosonMediaScanner\qmldir
  SetOutPath "$INSTDIR\NosonThumbnailer"
    File Files\NosonThumbnailer\NosonThumbnailer.dll
    File Files\NosonThumbnailer\qmldir
  ;END Noson Files

  ;BEGIN Qt Files
  SetOutPath "$INSTDIR"
    File Files\Qt5Core.dll
    File Files\Qt5DBus.dll
    File Files\Qt5Gui.dll
    File Files\Qt5Network.dll
    File Files\Qt5OpenGL.dll
    File Files\Qt5PrintSupport.dll
    File Files\Qt5Qml.dll
    File Files\Qt5Quick.dll
    File Files\Qt5QuickControls2.dll
    File Files\Qt5QuickTemplates2.dll
    File Files\Qt5QuickWidgets.dll
    File Files\Qt5Svg.dll
    File Files\Qt5Widgets.dll
    File Files\Qt5Xml.dll
    File Files\libEGL.dll
    File Files\libGLESv2.dll

  SetOutPath "$INSTDIR\bearer\"
    File Files\bearer\qgenericbearer.dll

  SetOutPath "$INSTDIR\iconengines\"
    File Files\iconengines\qsvgicon.dll

  SetOutPath "$INSTDIR\imageformats\"
    File Files\imageformats\qgif.dll
    File Files\imageformats\qicns.dll
    File Files\imageformats\qico.dll
    File Files\imageformats\qjpeg.dll
    File Files\imageformats\qsvg.dll
    File Files\imageformats\qtga.dll
    File Files\imageformats\qtiff.dll

  SetOutPath "$INSTDIR\platforms\"
    File Files\platforms\qwindows.dll

  SetOutPath "$INSTDIR\printsupport\"
    File Files\printsupport\windowsprintersupport.dll

  SetOutPath "$INSTDIR\translations"
    File Files\translations\qt*.qm

  SetOutPath "$INSTDIR"
    File /r Files\Qt
    File /r Files\QtGraphicalEffects
    File /r Files\QtQml
    File /r Files\QtQuick
    File /r Files\QtQuick.2
  ;END Qt Files

  ;BEGIN additional Files
  SetOutPath "$INSTDIR"
    File Files\qt.conf
  ;END additional Files


  ;the last "SetOutPath" will be the default directory
  SetOutPath "$INSTDIR"


  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd
LangString DESC_Noson ${LANG_ENGLISH} "Install the program files"


Section "Start Menu" StartMenu

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Noson.lnk" "$INSTDIR\noson-gui.exe" "" "$INSTDIR\noson.ico"
   !insertmacro MUI_STARTMENU_WRITE_END

  ;Create registry entries
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Noson" "DisplayName" "Noson"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Noson" "DisplayIcon" "$INSTDIR\noson.ico"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Noson" "UninstallString" "$INSTDIR\Uninstall.exe"

SectionEnd
LangString DESC_StartMenu ${LANG_ENGLISH} "Create Start Menu (deselect if you want install Noson as portable app)"


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
   !insertmacro MUI_DESCRIPTION_TEXT ${Noson} $(DESC_Noson)
   !insertmacro MUI_DESCRIPTION_TEXT ${StartMenu} $(DESC_StartMenu)
   !insertmacro MUI_DESCRIPTION_TEXT ${MSVC} $(DESC_MSVC)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;------------------------------------------------------------------------
;Uninstaller Sections                                                    -
;------------------------------------------------------------------------
Section "Uninstall"

  ;Install for all users
  SetShellVarContext all

  Delete "$INSTDIR\Uninstall.exe"

  SetOutPath $TEMP

  RMDir /r $INSTDIR

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Noson.lnk"

  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey /ifempty HKCU "Software\Noson"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Noson"

SectionEnd

Function .onInit
  # set section 'MSVC' as unselected
  #SectionSetFlags ${MSVC} 0
FunctionEnd


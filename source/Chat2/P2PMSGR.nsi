Outfile "chat2inst.exe"
InstallDir "$PROGRAMFILES\Chat2"
RequestExecutionLevel admin

VIProductVersion "1.0.0.0"
VIFileVersion "1.0.0.0"
VIAddVersionKey /LANG=1033 "FileVersion" "1.0.0.0"
VIAddVersionKey /LANG=1033 "ProductVersion" "1.0.0.0"
VIAddVersionKey /LANG=1033 "FileDescription" "Chat2 Installer"
VIAddVersionKey /LANG=1033 "InternalName" "chat2inst.exe"
VIAddVersionKey /LANG=1033 "OriginalFilename" "chat2inst.exe"
VIAddVersionKey /LANG=1033 "CompanyName" "-"
VIAddVersionKey /LANG=1033 "LegalCopyright" "© WinXP655, 2025"
VIAddVersionKey /LANG=1033 "ProductName" "Chat2"

Name "Chat2"
Caption "Chat2 Setup"
BrandingText "P2P LAN Messenger"

!include "MUI2.nsh"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Section "Splash"
  AdvSplash::Show /NOUNLOAD /TIME=3000 /TEXT="Initializing Chat2 installer..."
  Pop $0
SectionEnd

Section "Install"

  SetOutPath "$INSTDIR"
  File "chat2.exe"
  File "Join.wav"
  File "Left.wav"
  File "newMsg.wav"
  File "Readme.txt"

  ; Create Start Menu shortcut in selected folder
  CreateDirectory "$SMPROGRAMS\Chat2"
  CreateShortCut "$SMPROGRAMS\Chat2\Chat2.lnk" "$INSTDIR\chat2.exe"
  CreateShortCut "$SMPROGRAMS\Chat2\Readme.lnk" "$INSTDIR\readme.txt"
  WriteUninstaller "$INSTDIR\uninst.exe"
  CreateShortCut "$SMPROGRAMS\Chat2\Uninstall.lnk" "$INSTDIR\uninst.exe"

  ; Registry info for uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chat2" "DisplayName" "Chat2"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chat2" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chat2" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chat2" "DisplayIcon" "$INSTDIR\chat2.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chat2" "Publisher" "WinXP655"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chat2" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chat2" "NoRepair" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chat2" "EstimatedSize" 348

SectionEnd

Section "Uninstall"

  Delete "$INSTDIR\chat2.exe"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\join.wav"
  Delete "$INSTDIR\left.wav"
  Delete "$INSTDIR\newmsg.wav"
  Delete "$INSTDIR\uninst.exe"
  RMDir "$INSTDIR"

  Delete "$SMPROGRAMS\Chat2\Chat2.lnk"
  Delete "$SMPROGRAMS\Chat2\Readme.lnk"
  Delete "$SMPROGRAMS\Chat2\Uninstall.lnk"
  RMDir "$SMPROGRAMS\Chat2"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Chat2"

SectionEnd
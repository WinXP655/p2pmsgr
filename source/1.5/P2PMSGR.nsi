Icon "setup.ico"
Outfile "p2pmsgrinst.exe"
InstallDir "$PROGRAMFILES\P2P Messenger"
RequestExecutionLevel admin

VIProductVersion "1.0.0.0"
VIFileVersion "1.0.0.0"
VIAddVersionKey /LANG=1033 "FileVersion" "1.0.0.0"
VIAddVersionKey /LANG=1033 "ProductVersion" "1.0.0.0"
VIAddVersionKey /LANG=1033 "FileDescription" "P2P Messenger Installer"
VIAddVersionKey /LANG=1033 "InternalName" "p2pmsgrinst.exe"
VIAddVersionKey /LANG=1033 "OriginalFilename" "p2pmsgrinst.exe"
VIAddVersionKey /LANG=1033 "CompanyName" "-"
VIAddVersionKey /LANG=1033 "LegalCopyright" "© WinXP655, 2025"
VIAddVersionKey /LANG=1033 "ProductName" "P2P Messenger"

Name "P2P Messenger"
Caption "P2P Messenger Setup"
BrandingText "P2P LAN Messenger"

!include "MUI2.nsh"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Section "Splash"
  AdvSplash::Show /NOUNLOAD /TIME=3000 /TEXT="Initializing P2P Messenger installer..."
  Pop $0
SectionEnd

Section "Install"

  SetOutPath "$INSTDIR"
  File "p2pmsgr.exe"
  File "Error.wav"
  File "Join.wav"
  File "Left.wav"
  File "Msg.wav"
  File "Sys.wav"
  File "Readme.txt"

  ; Create Start Menu shortcut in selected folder
  CreateDirectory "$SMPROGRAMS\P2P Messenger"
  CreateShortCut "$SMPROGRAMS\P2P Messenger\P2P Messenger.lnk" "$INSTDIR\p2pmsgr.exe"
  CreateShortCut "$SMPROGRAMS\P2P Messenger\Readme.lnk" "$INSTDIR\readme.txt"
  WriteUninstaller "$INSTDIR\uninst.exe"
  CreateShortCut "$SMPROGRAMS\P2P Messenger\Uninstall.lnk" "$INSTDIR\uninst.exe"

  ; Registry info for uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\P2P Messenger" "DisplayName" "P2P Messenger"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\P2P Messenger" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\P2P Messenger" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\P2P Messenger" "DisplayIcon" "$INSTDIR\p2pmsgr.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\P2P Messenger" "Publisher" "WinXP655"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\P2P Messenger" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\P2P Messenger" "NoRepair" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\P2P Messenger" "EstimatedSize" 348

SectionEnd

Section "Uninstall"

  Delete "$INSTDIR\p2pmsgr.exe"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\error.wav"
  Delete "$INSTDIR\join.wav"
  Delete "$INSTDIR\left.wav"
  Delete "$INSTDIR\msg.wav"
  Delete "$INSTDIR\sys.wav"
  Delete "$INSTDIR\uninst.exe"
  RMDir "$INSTDIR"

  Delete "$SMPROGRAMS\P2P Messenger\P2P Messenger.lnk"
  Delete "$SMPROGRAMS\P2P Messenger\Readme.lnk"
  Delete "$SMPROGRAMS\P2P Messenger\Uninstall.lnk"
  RMDir "$SMPROGRAMS\P2P Messenger"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\P2P Messenger"

SectionEnd
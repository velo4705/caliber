; Caliber — NSIS Installer Script
; Produces: Caliber-1.1.0-Setup.exe
;
; HOW TO BUILD:
;   This script lives in the "windows\" subfolder inside the extracted zip.
;   The exe and DLLs are in the parent folder (..\).
;   Open a command prompt in the "windows\" folder and run:
;       makensis installer.nsi
;
; Requires: NSIS 3.x  (https://nsis.sourceforge.io)

Unicode True

!define APP_NAME        "Caliber"
!define APP_VERSION     "1.1.0"
!define APP_PUBLISHER   "Caliber"
!define APP_EXE         "caliber.exe"
!define INSTALL_DIR     "$PROGRAMFILES64\Caliber"
!define UNINSTALL_KEY   "Software\Microsoft\Windows\CurrentVersion\Uninstall\Caliber"

; Files are in the parent folder (one level up from this script)
!define DEPLOY_DIR      ".."

Name            "${APP_NAME} ${APP_VERSION}"
OutFile         "Caliber-${APP_VERSION}-Setup.exe"
InstallDir      "${INSTALL_DIR}"
InstallDirRegKey HKLM "${UNINSTALL_KEY}" "InstallLocation"
RequestExecutionLevel admin
SetCompressor   /SOLID lzma

; ── Modern UI ─────────────────────────────────────────────────────────────────
!include "MUI2.nsh"

!define MUI_ICON            "${DEPLOY_DIR}\caliber.ico"
!define MUI_UNICON          "${DEPLOY_DIR}\caliber.ico"
!define MUI_FINISHPAGE_RUN  "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Launch Caliber"
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; ── Install ───────────────────────────────────────────────────────────────────
Section "Caliber" SecMain
    SetOutPath "$INSTDIR"

    ; Copy caliber.exe and all DLLs from parent folder
    File "${DEPLOY_DIR}\caliber.exe"
    File "${DEPLOY_DIR}\caliber.ico"
    File "${DEPLOY_DIR}\*.dll"

    ; Copy Qt plugin subfolders
    File /r "${DEPLOY_DIR}\platforms"
    File /r "${DEPLOY_DIR}\styles"
    File /r "${DEPLOY_DIR}\imageformats"
    File /r "${DEPLOY_DIR}\tls"

    ; Desktop shortcut — icon from exe (multi-size, no graininess)
    CreateShortcut  "$DESKTOP\Caliber.lnk" \
                    "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0

    ; Start Menu shortcuts
    CreateDirectory "$SMPROGRAMS\Caliber"
    CreateShortcut  "$SMPROGRAMS\Caliber\Caliber.lnk" \
                    "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0
    CreateShortcut  "$SMPROGRAMS\Caliber\Uninstall Caliber.lnk" \
                    "$INSTDIR\Uninstall.exe"

    ; Register with Windows Programs and Features
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayName"          "${APP_NAME}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayVersion"       "${APP_VERSION}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "Publisher"            "${APP_PUBLISHER}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "InstallLocation"      "$INSTDIR"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "DisplayIcon"          "$INSTDIR\${APP_EXE}"
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "UninstallString"      '"$INSTDIR\Uninstall.exe"'
    WriteRegStr   HKLM "${UNINSTALL_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify"             1
    WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair"             1

    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

; ── Uninstall ─────────────────────────────────────────────────────────────────
Section "Uninstall"
    Delete "$SMPROGRAMS\Caliber\Caliber.lnk"
    Delete "$SMPROGRAMS\Caliber\Uninstall Caliber.lnk"
    RMDir  "$SMPROGRAMS\Caliber"
    Delete "$DESKTOP\Caliber.lnk"

    RMDir /r "$INSTDIR"

    DeleteRegKey HKLM "${UNINSTALL_KEY}"
SectionEnd

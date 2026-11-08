#define MyAppName "KEYFORGE"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "KEYFORGE"
#define MyAppExeName "KEYFORGE.exe"

[Setup]
AppId={{7B2F1E0A-9B8B-4A77-A8AF-6F5A0D3B91C2}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
DefaultDirName={localappdata}\Programs\KEYFORGE
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir=..\release
OutputBaseFilename=KEYFORGE-Setup-{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Files]
Source: "..\build\Release\KEYFORGE.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\sounds\*"; DestDir: "{app}\sounds"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\SECURITY.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\KEYFORGE"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\KEYFORGE"; Filename: "{app}\{#MyAppExeName}"

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

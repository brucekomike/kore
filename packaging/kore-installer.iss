#ifndef MyAppVersion
  #define MyAppVersion "0.1.0"
#endif

#ifndef SourceDir
  #error "SourceDir preprocessor define is required"
#endif

#ifndef OutputDir
  #error "OutputDir preprocessor define is required"
#endif

[Setup]
AppName=kore
AppVersion={#MyAppVersion}
DefaultDirName={autopf}\kore
DefaultGroupName=kore
OutputDir={#OutputDir}
OutputBaseFilename=kore-windows
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{autoprograms}\kore"; Filename: "{app}\kore.exe"
Name: "{autodesktop}\kore"; Filename: "{app}\kore.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\kore.exe"; Description: "Launch kore"; Flags: nowait postinstall skipifsilent

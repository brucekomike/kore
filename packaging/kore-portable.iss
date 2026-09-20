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
AppName=kore Portable
AppVersion={#MyAppVersion}
DefaultDirName={code:GetDefaultPortableDir}
OutputDir={#OutputDir}
OutputBaseFilename=kore-windows-portable
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
DisableProgramGroupPage=yes
CreateUninstallRegKey=no
Uninstallable=no

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Code]
function GetDefaultPortableDir(Param: string): string;
begin
  Result := ExpandConstant('{src}\kore-portable');
end;

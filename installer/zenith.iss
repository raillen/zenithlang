; Zenith installer (Inno Setup)
; Build with:
;   iscc installer\zenith.iss /DMyAppVersion=0.3.0-alpha.2 /DSourceDir=C:\path\to\stage /DOutputDir=C:\path\to\dist\installer

#ifndef MyAppName
  #define MyAppName "Zenith Language"
#endif

#ifndef MyAppVersion
  #define MyAppVersion "0.3.0-alpha.1"
#endif

#ifndef MyAppPublisher
  #define MyAppPublisher "Zenith Language Project"
#endif

#ifndef MyAppURL
  #define MyAppURL "https://github.com/raillen/zenithlang"
#endif

; Keep AppId stable between versions.
#ifndef MyAppId
  #define MyAppId "{{3C6534BA-5F34-4AB7-B4E8-E8D2A7F2A6B7}"
#endif

#ifndef SourceDir
  ; Prefer staged release payload when available.
  ; Fallback to repository root for direct IDE/manual compilation.
  #ifexist "..\.artifacts\installer\stage\zt.exe"
    #define SourceDir "..\.artifacts\installer\stage"
  #else
    #define SourceDir ".."
  #endif
#endif

#ifndef OutputDir
  #define OutputDir "..\dist\installer"
#endif

#ifndef OutputBase
  #define OutputBase "zenith-" + MyAppVersion + "-windows-amd64-setup"
#endif

#ifndef InstallerIconPath
  #define StageIconPath SourceDir + "\logo-only.ico"
  #ifexist StageIconPath
    #define InstallerIconPath StageIconPath
  #else
    #define InstallerIconPath "..\branding\logo-only.ico"
  #endif
#endif

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\Zenith
DefaultGroupName=Zenith
DisableProgramGroupPage=yes
LicenseFile={#SourceDir}\LICENSE
OutputDir={#OutputDir}
OutputBaseFilename={#OutputBase}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
ChangesEnvironment=yes
SetupIconFile={#InstallerIconPath}
UninstallDisplayIcon={app}\logo-only.ico

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "addtopath"; Description: "Add Zenith to system PATH"; Flags: checkedonce

[Files]
Source: "{#SourceDir}\zt.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#InstallerIconPath}"; DestDir: "{app}"; DestName: "logo-only.ico"; Flags: ignoreversion
Source: "{#SourceDir}\runtime\*"; DestDir: "{app}\runtime"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#SourceDir}\stdlib\*"; DestDir: "{app}\stdlib"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#SourceDir}\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\LICENSE-APACHE"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\LICENSE-MIT"; DestDir: "{app}"; Flags: ignoreversion

[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "ZENITH_HOME"; ValueData: "{app}"; Flags: uninsdeletevalue

[Code]
const
  EnvRegKey = 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment';
  EnvPathName = 'Path';

function PathContains(const FullPath, Item: string): Boolean;
var
  ExpandedPath: string;
  ExpandedItem: string;
begin
  ExpandedPath := ';' + LowerCase(FullPath) + ';';
  ExpandedItem := ';' + LowerCase(Item) + ';';
  Result := Pos(ExpandedItem, ExpandedPath) > 0;
end;

procedure AddInstallDirToPath;
var
  CurrentPath: string;
  InstallPath: string;
begin
  InstallPath := ExpandConstant('{app}');
  if not RegQueryStringValue(HKLM, EnvRegKey, EnvPathName, CurrentPath) then begin
    CurrentPath := '';
  end;

  StringChangeEx(CurrentPath, ';;', ';', True);
  if not PathContains(CurrentPath, InstallPath) then begin
    if (CurrentPath <> '') and (Copy(CurrentPath, Length(CurrentPath), 1) <> ';') then begin
      CurrentPath := CurrentPath + ';';
    end;
    CurrentPath := CurrentPath + InstallPath;
    RegWriteExpandStringValue(HKLM, EnvRegKey, EnvPathName, CurrentPath);
  end;
end;

procedure RemoveInstallDirFromPath;
var
  CurrentPath: string;
  InstallPath: string;
  UpdatedPath: string;
begin
  InstallPath := ExpandConstant('{app}');
  if not RegQueryStringValue(HKLM, EnvRegKey, EnvPathName, CurrentPath) then begin
    exit;
  end;

  UpdatedPath := CurrentPath;
  StringChangeEx(UpdatedPath, ';' + InstallPath, '', True);
  StringChangeEx(UpdatedPath, InstallPath + ';', '', True);
  StringChangeEx(UpdatedPath, InstallPath, '', True);
  StringChangeEx(UpdatedPath, ';;', ';', True);

  if (UpdatedPath <> '') and (Copy(UpdatedPath, 1, 1) = ';') then begin
    Delete(UpdatedPath, 1, 1);
  end;
  if (UpdatedPath <> '') and (Copy(UpdatedPath, Length(UpdatedPath), 1) = ';') then begin
    Delete(UpdatedPath, Length(UpdatedPath), 1);
  end;

  RegWriteExpandStringValue(HKLM, EnvRegKey, EnvPathName, UpdatedPath);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) and WizardIsTaskSelected('addtopath') then begin
    AddInstallDirToPath;
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then begin
    RemoveInstallDirFromPath;
  end;
end;

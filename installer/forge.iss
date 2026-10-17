; The Forge installer (Inno Setup 6). Built by installer\build.ps1, which puts everything it installs in
; installer\stage first and passes the version:
;
;   iscc /DVersion=0.9.4 installer\forge.iss
;
; It installs for the current Windows user only (no admin rights needed), into
; %LOCALAPPDATA%\Programs\Forge: frg.exe and its runtime, its own C++ compiler (w64devkit, which frg uses by
; itself), the docs, the examples and the VS Code extension. Running a newer installer updates it in place.
; Choices: Forge on the PATH, .frg file icons, and the VS Code extension. Uninstalling undoes all of them.

#ifndef Version
  #define Version "0.0.0"
#endif

[Setup]
AppId={{6F2B8E61-3C1D-4D3A-9E7B-0F5A2C9D4E18}
AppName=Forge
AppVersion={#Version}
AppVerName=Forge {#Version}
AppPublisher=ChainzYTTTV
AppPublisherURL=https://github.com/ChainzYTTTV/frg
AppSupportURL=https://github.com/ChainzYTTTV/frg/issues
AppUpdatesURL=https://github.com/ChainzYTTTV/frg/releases
DefaultDirName={localappdata}\Programs\Forge
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=out
OutputBaseFilename=Forge-{#Version}-setup
SetupIconFile=..\art\frg-file.ico
UninstallDisplayIcon={app}\art\frg-file.ico
UninstallDisplayName=Forge {#Version}
LicenseFile=..\LICENSE
WizardStyle=modern
Compression=lzma2/max
SolidCompression=yes
ChangesEnvironment=yes
ChangesAssociations=yes

[Tasks]
Name: "path"; Description: "Add Forge to my PATH, so frg works in any terminal"
Name: "fileicons"; Description: "Give .frg files the Forge icon (and open them in VS Code, if it's installed)"
Name: "vscode"; Description: "Install the Forge extension for VS Code"; Check: HasVSCode

[InstallDelete]
; an update replaces the runtime completely, so no old file stays behind
Type: filesandordirs; Name: "{app}\bin\runtime"

[Files]
Source: "stage\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[UninstallDelete]
; made by frg as it's used: the build cache and the update check's note
Type: filesandordirs; Name: "{app}\bin\cache"
Type: files; Name: "{app}\bin\update.txt"

[Registry]
Root: HKCU; Subkey: "Software\Classes\.frg"; ValueType: string; ValueName: ""; ValueData: "Forge.SourceFile"; Flags: uninsdeletevalue; Tasks: fileicons
Root: HKCU; Subkey: "Software\Classes\.frg"; ValueType: string; ValueName: "PerceivedType"; ValueData: "text"; Flags: uninsdeletevalue; Tasks: fileicons
Root: HKCU; Subkey: "Software\Classes\Forge.SourceFile"; ValueType: string; ValueName: ""; ValueData: "Forge source file"; Flags: uninsdeletekey; Tasks: fileicons
Root: HKCU; Subkey: "Software\Classes\Forge.SourceFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: """{app}\art\frg-file.ico"""; Tasks: fileicons
Root: HKCU; Subkey: "Software\Classes\Forge.SourceFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{code:CodeExe}"" ""%1"""; Tasks: fileicons; Check: HasVSCode

[Run]
Filename: "{cmd}"; Parameters: "/c """"{code:CodeCmd}"" --install-extension ""{app}\editors\forge.vsix"" --force"""; Flags: runhidden waituntilterminated; Tasks: vscode; StatusMsg: "Installing the VS Code extension..."
Filename: "{app}\docs\getting-started.md"; Description: "Open the getting started guide"; Flags: postinstall shellexec skipifsilent unchecked

[Code]
function CodeDir(): String;
begin
  Result := ExpandConstant('{localappdata}\Programs\Microsoft VS Code');
  if not FileExists(Result + '\Code.exe') then
    Result := ExpandConstant('{commonpf64}\Microsoft VS Code');
  if not FileExists(Result + '\Code.exe') then
    Result := '';
end;

function HasVSCode(): Boolean;
begin
  Result := CodeDir() <> '';
end;

function CodeExe(Param: String): String;
begin
  Result := CodeDir() + '\Code.exe';
end;

function CodeCmd(Param: String): String;
begin
  Result := CodeDir() + '\bin\code.cmd';
end;

// The user's PATH (HKCU\Environment), with Forge's bin folder added or taken out.
const
  EnvKey = 'Environment';

function BinDir(): String;
begin
  Result := ExpandConstant('{app}\bin');
end;

function PathHas(Path, Dir: String): Boolean;
begin
  Result := Pos(';' + Uppercase(Dir) + ';', ';' + Uppercase(Path) + ';') > 0;
end;

procedure AddToPath();
var
  Path: String;
begin
  if not RegQueryStringValue(HKCU, EnvKey, 'Path', Path) then
    Path := '';
  if PathHas(Path, BinDir()) then
    exit;
  if (Path <> '') and (Copy(Path, Length(Path), 1) <> ';') then
    Path := Path + ';';
  RegWriteExpandStringValue(HKCU, EnvKey, 'Path', Path + BinDir());
end;

procedure RemoveFromPath();
var
  Path, Upper, Dir: String;
  At: Integer;
begin
  if not RegQueryStringValue(HKCU, EnvKey, 'Path', Path) then
    exit;
  Dir := Uppercase(BinDir());
  Path := ';' + Path + ';';
  Upper := Uppercase(Path);
  At := Pos(';' + Dir + ';', Upper);
  if At = 0 then
    exit;
  Delete(Path, At, Length(Dir) + 1);
  // trim the ; added at both ends
  if Copy(Path, 1, 1) = ';' then
    Delete(Path, 1, 1);
  if Copy(Path, Length(Path), 1) = ';' then
    Delete(Path, Length(Path), 1);
  RegWriteExpandStringValue(HKCU, EnvKey, 'Path', Path);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) and WizardIsTaskSelected('path') then
    AddToPath();
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
    RemoveFromPath();
end;

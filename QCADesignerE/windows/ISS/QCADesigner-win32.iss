; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=QCADesignerE
AppVerName=QCADesignerE 2.2
AppPublisher=University of Calgary/University of Bremen
AppPublisherURL=https://github.com/FSillT/QCADesigner-E
AppSupportURL=https://github.com/FSillT/QCADesigner-E
AppUpdatesURL=https://github.com/FSillT/QCADesigner-E
DefaultDirName={pf}\QCADesignerE
DefaultGroupName=QCADesignerE
LicenseFile=d:\data\Dev\QCA\QCAEnergy\windows\QCADesigner\share\doc\QCADesigner-2.2\COPYING
ChangesAssociations=yes
OutputBaseFilename=QCADesignerE-2.2-setup

[Tasks]
; NOTE: The following entry contains English phrases ("Create a desktop icon" and "Additional icons"). You are free to translate them into another language if required.
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Registry]
; This adds the GTK+ libraries to QCADesignerE.exe's path
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\QCADesignerE.exe"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\QCADesignerE.exe"; ValueType: string; ValueData: "{app}\bin\QCADesignerE.exe"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\QCADesignerE.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app};{code:GetGtkPath}\bin"; Flags: uninsdeletevalue
; This adds the GTK+ libraries to graph_dialog.exe's path
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\graph_dialog.exe"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\graph_dialog.exe"; ValueType: string; ValueData: "{app}\bin\QCADesignerE.exe"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\graph_dialog.exe"; ValueType: string; ValueName: "Path"; ValueData: "{app};{code:GetGtkPath}\bin"; Flags: uninsdeletevalue
; File associations
; QCA Design
Root: HKCR; SubKey: ".qca"; Flags: uninsdeletekey
Root: HKCR; SubKey: ".qca"; ValueType: string; ValueName: ""; ValueData: "QCADesignerE.File"
Root: HKCR; SubKey: "QCADesignerE.File"; Flags: uninsdeletekey
Root: HKCR; SubKey: "QCADesignerE.File"; ValueType: string; ValueName: ""; ValueData: "QCADesignerE Design"
Root: HKCR; SubKey: "QCADesignerE.File"; ValueType: dword; ValueName: "BrowserFlags"; ValueData: 8; Flags: uninsdeletevalue
Root: HKCR; SubKey: "QCADesignerE.File"; ValueType: dword; ValueName: "EditFlags"; ValueData: 0; Flags: uninsdeletevalue
Root: HKCR; SubKey: "QCADesignerE.File\DefaultIcon"; Flags: uninsdeletekey
Root: HKCR; SubKey: "QCADesignerE.File\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\share\QCADesignerE\pixmaps\QCADesignerE.winxp.ico"; MinVersion: 0, 1
Root: HKCR; SubKey: "QCADesignerE.File\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\share\QCADesignerE\pixmaps\QCADesignerE.win32.ico"; MinVersion: 1, 0
Root: HKCR; SubKey: "QCADesignerE.File\shell"; Flags: uninsdeletekey
Root: HKCR; SubKey: "QCADesignerE.File\shell"; ValueType: string; ValueName: ""; ValueData: "Open"
Root: HKCR; SubKey: "QCADesignerE.File\shell\Open"; Flags: uninsdeletekey
Root: HKCR; SubKey: "QCADesignerE.File\shell\Open"; ValueType: string; ValueName: ""; ValueData: "&Open"
Root: HKCR; SubKey: "QCADesignerE.File\shell\Open\command"; Flags: uninsdeletekey
Root: HKCR; SubKey: "QCADesignerE.File\shell\Open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\QCADesignerE.exe"" ""%1"""

[Files]
Source: "d:\data\Dev\QCA\QCAEnergy\windows\QCADesigner\*"; DestDir: "{app}"; Flags: recursesubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\QCADesignerE"; Filename: "{app}\bin\QCADesignerE.exe"; IconFilename: "{app}\share\QCADesignerE\pixmaps\QCADesignerE.winxp.ico"; MinVersion: 0, 1
Name: "{userdesktop}\QCADesignerE"; Filename: "{app}\bin\QCADesignerE.exe"; Tasks: desktopicon; IconFilename: "{app}\share\QCADesignerE\pixmaps\QCADesignerE.winxp.ico"; MinVersion: 0, 1

Name: "{group}\QCADesigner"; Filename: "{app}\bin\QCADesignerE.exe"; IconFilename: "{app}\share\QCADesignerE\pixmaps\QCADesignerE.win32.ico"; MinVersion: 1, 0
Name: "{group}\Simulation Results Viewer"; Filename: "{app}\bin\graph_dialog.exe"; IconFilename: "{app}\share\QCADesignerE\pixmaps\graph_dialog.win32.ico"; MinVersion: 1, 0
Name: "{userdesktop}\QCADesignerE"; Filename: "{app}\bin\QCADesignerE.exe"; Tasks: desktopicon; IconFilename: "{app}\share\QCADesignerE\pixmaps\QCADesignerE.win32.ico"; MinVersion: 1, 0

[Code]

var
  Exists: Boolean;
  GtkPath: String;

function GetGtkInstalled (): Boolean;
begin
  Exists := RegQueryStringValue (HKLM, 'Software\GTK\2.2', 'Path', GtkPath);
  if not Exists then begin
    Exists := RegQueryStringValue (HKCU, 'Software\GTK\2.2', 'Path', GtkPath);
  end;
   Result := Exists
end;

function GetGtkPath (S: String): String;
begin
    Result := GtkPath;
end;

function InitializeSetup(): Boolean;
begin
  Result := GetGtkInstalled ();
  if not Result then begin
    MsgBox ('Please install the GTK+ Runtime Environment version 2.2. You can obtain GTK+ from https://sourceforge.net/projects/gtk-win/files/latest/download.', mbInformation, MB_OK);
  end;
  Result := true;
end;

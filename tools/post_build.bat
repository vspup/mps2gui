@RD /S /Q "%3/EXE"

rem # QT cmd location
C:\Windows\System32\cmd.exe /A /Q /K C:\Qt\6.3.2\mingw_64\bin\qtenv2.bat

rem # arguments: %{ActiveProject::QT_HOST_LIBEXECS}\windeployqt.exe %{ActiveProject:RunConfig:Executable:NativeFilePath} %{ActiveProject:RunConfig:Executable:NativePath}
%1 %2 --dir %3/EXE

xcopy %3\libnng.dll "%3/EXE"
xcopy %2 "%3/EXE"
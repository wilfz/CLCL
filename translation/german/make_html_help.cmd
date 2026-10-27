set SCRIPTDIR=%~dp0
set WORKDIR=%SCRIPTDIR%
cd %WORKDIR%

if not exist [..\..\Release] mkdir ..\..\Release
:: generate clcl.chm using pandoc.exe and "%ProgramFiles(x86)%\\HTML Help Workshop\\hhc.exe"
python.exe ..\..\..\md2chm\md2chm.py README_de.md alias.h ..\..\img --target clcl_de -w ..\..\Release --title "CLCL Clipboard Manager" --default_topic #übersicht --map ..\..\resource.h --map ..\..\CLCLSet\resource.h
copy ..\..\Release\clcl_de.chm ..\..\Documentation\
cd %SCRIPTDIR%
pause

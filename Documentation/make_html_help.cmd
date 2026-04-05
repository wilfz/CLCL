set SCRIPTDIR=%~dp0
set WORKDIR=%SCRIPTDIR%\..
cd %WORKDIR%

:: clean up
rmdir /S /Q CLCL_help
:: generate clcl.chm using pandoc.exe and "%ProgramFiles(x86)%\\HTML Help Workshop\\hhc.exe"
python.exe ..\md2chm\md2chm.py README.md Documentation\alias.h img --target clcl -w CLCL_help --title "CLCL Documentation" --default_topic #overview --map resource.h --map CLCLSet\resource.h
copy CLCL_help\clcl.chm .\Documentation\
cd %SCRIPTDIR%

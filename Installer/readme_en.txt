## CLCL Installer

■ Overview
This is the CLCL installer and the tool to build it.

The installer is written entirely in C and does not use any libraries other than the Windows API. Since the ZIP extraction process (deflate) is also implemented directly inside the installer, no runtime or external libraries are required at execution time.

■ Files
Installer.vcxproj   Installer project (included in CLCL.sln)
build.ps1           PowerShell script to build the installer
prepare.ps1         Collects files and creates the ZIP (called from the project)
Installer.c         Main installer code
Installer.h         Installer definitions
Installer.rc        Resources (dialogs / localized strings / ZIP)
resource.h          Resource IDs
unzip.c             ZIP extraction (deflate and CRC32 implementation)
unzip.h             ZIP extraction definitions
res/manifest.xml    Manifest (runs with administrator privileges)

■ Building the Installer
Visual Studio 2017 or later (Desktop development with C++) and the Windows SDK are required.
Running the following command in PowerShell will handle everything from building CLCL itself to generating the installer in a single pass:

PS> cd Installer
PS> .\build.ps1

The generated installer is saved to `Installer\out`.
The output filename will be `clcl<version>.exe`. For example, Ver 2.2.0 will produce `clcl220.exe`. The version number is retrieved from `FILEVERSION` in `CLCL.rc`.
The installer bundles `CLCL.exe` and other assets built under the Release configuration.

To build only the installer using existing built files, pass `-SkipBuild`:

PS> .\build.ps1 -SkipBuild

Main Options:
-Configuration     Build configuration (Default: Release)
-Platform          Target platform to build (Default: x86)
-Version           Version string (Default: FILEVERSION from CLCL.rc)
-SourceDir         Source folder to collect files from (Default: <Configuration>)
-OutDir            Output destination folder (Default: Installer\out)
-SkipBuild         Skips building CLCL itself

■ Building in Visual Studio
`Installer.vcxproj` is included in `CLCL.sln`. Building the solution will generate the installer right after CLCL itself is built. You can also build only the `Installer` project, provided that CLCL itself has already been built.

In the pre-build event, `prepare.ps1` gathers the files, creates the ZIP archive, and generates `instinfo.h` containing version information. In the post-build event, `prepare.ps1` copies the output to `Installer\out` as `clcl<version>.exe`. Since `build.ps1` simply invokes `msbuild` on this project, both build methods produce identical results.

Output is saved to `Installer\out` only when building with the Release configuration. Installers built using the Debug configuration will be placed in `Installer\Debug\CLCLInst.exe`. Because Debug builds collect Debug binaries (such as Debug `CLCL.exe`), this prevents them from being mixed up with distributable installers.

Project settings can be changed via msbuild properties. The parameters in `build.ps1` correspond to these properties. Please do not append trailing backslashes (`\`) to folder paths.
ClclVersion     Version
ClclSourceDir   Source location of files to install
ClclOutDir      Installer output directory

The version information for the installer is determined by `instinfo.h`, which is generated during the build process. This file is not included in the repository.

■ Installed Files
`prepare.ps1` collects the following files, packs them into a ZIP archive, and embeds it as an installer resource (`RCDATA`). At runtime, the installer extracts and deploys this ZIP archive.

CLCL.exe        Collected from the Release folder
CLCLSet.exe     Collected from the Release folder
CLCLHook.dll    Collected from the Release folder
readme_jp.txt   Collected from the repository root
readme_en.txt
readme_de.txt
readme_uk.txt
readme_zh.txt
LICENSE.txt     Collected from LICENSE in the repository

`clcl_app.ini` is intentionally omitted from collection. If the file is missing, the app operates as `portable=0`, ensuring user settings are not overwritten during upgrades.

To modify which files are packaged, edit `Get-TargetFiles` inside `prepare.ps1`.

■ Installer Behavior
Runs with Administrator privileges (specified as `requireAdministrator` in the manifest).

・You can select the installation path.
The default path is `%ProgramFiles%\CLCL`. If an existing installation is detected, its registered path becomes the default.
・Options are provided to create shortcuts in Startup, the Start Menu, and the Desktop. Shortcuts are created in all-users locations.
・Registers the application under "Programs and Features" (Apps & features) in Control Panel.
・Updates the installation if CLCL is already present.
It reuses the existing registry key in the registered applications list to avoid duplicate entries.
・If CLCL is running and files cannot be overwritten, it prompts the user to close CLCL.
・Prompts whether to launch CLCL after installation completes. If launched, it runs via Explorer to avoid inheriting administrator privileges.

■ Uninstalling
Can be executed from the Control Panel.
You can also run the installer executable with command-line arguments:

clcl220.exe /uninstall

The `uninstall.exe` placed in the installation folder exhibits the exact same behavior:

"C:\Program Files (x86)\CLCL\uninstall.exe" /uninstall

A list of installed files is recorded in `uninstall.dat` within the installation directory. Uninstalling removes the files and shortcuts specified in this list.
At execution time, the user is prompted whether to remove configuration and history data (`%LOCALAPPDATA%\CLCL`).

■ Application List Registration
Searches under the following registry keys for an entry whose display name starts with "CLCL". If found, that existing key is reused:

HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Uninstall
(Both 32-bit and 64-bit views)
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Uninstall

If not found, a new key is created at the following location:

HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Uninstall\CLCL
(Because it is a 32-bit application, it will reside under WOW6432Node on 64-bit systems)

Since this matches the key path used by Ver 2.1 installers, overwriting an existing environment will not create duplicate entries.

Registered Values:
DisplayName, DisplayVersion, DisplayIcon, Publisher, URLInfoAbout,
InstallLocation, UninstallString, InstallDate, NoModify, NoRepair,
EstimatedSize, VersionMajor, VersionMinor

All key values are deleted prior to writing new ones. This prevents residual values left by older installers (such as an `UninstallString` pointing to a different uninstaller) from causing the Control Panel to invoke an obsolete uninstaller.

If a file list left behind by an older installer (`install.DAT`) exists in the target directory, it is scheduled for deletion upon uninstallation to ensure the installation folder is thoroughly cleaned up.

If an existing installer uses a different key name, modify `UNINSTALL_SUBKEY` in `Installer.h`. If an existing registration is detected, that key is used regardless of name, so altering this is generally unnecessary.

■ Multilingual Support
Includes localized resources for the same 5 languages supported by CLCL itself:

Japanese / English / German / Chinese (Simplified) / Ukrainian

Switches automatically according to the Windows UI language setting. If a language resource is unavailable, it defaults to English. Dialog layouts are designed to be language-independent, with all displayed strings set dynamically at runtime from string resources.
To modify translations, edit the `STRINGTABLE` in `Installer.rc`.

■ License
Shares the same license as CLCL itself. Please refer to `LICENSE`.

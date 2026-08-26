#Requires -Version 5.1
<#
.SYNOPSIS
	CLCL のインストーラを作成します。

.DESCRIPTION
	CLCL 本体のビルド、インストールするファイルの収集、ZIP の作成、
	インストーラのコンパイルまでを一括で行います。
	出力するファイル名は clcl<バージョン>.exe になります。(Ver 2.2.0 なら clcl220.exe)

.EXAMPLE
	PS> .\build.ps1
	CLCL 本体をビルドしてからインストーラを作成します。

.EXAMPLE
	PS> .\build.ps1 -SkipBuild
	ビルド済みの Release フォルダのファイルからインストーラを作成します。

.EXAMPLE
	PS> .\build.ps1 -Version 2.2.1 -OutDir C:\temp
	バージョンと出力先を指定してインストーラを作成します。
#>
param(
	# ビルド構成
	[string]$Configuration = "Release",
	# ビルドするプラットフォーム
	[string]$Platform = "x86",
	# バージョン (省略時は CLCL.rc の FILEVERSION を使用)
	[string]$Version = "",
	# インストールするファイルの収集元 (省略時は <リポジトリ>\Release)
	[string]$SourceDir = "",
	# インストーラの出力先 (省略時は <リポジトリ>\Installer\out)
	[string]$OutDir = "",
	# CLCL 本体のビルドを行わない
	[switch]$SkipBuild,
	# 作業フォルダを残す
	[switch]$KeepWork
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Step([string]$message) {
	Write-Host "==> $message" -ForegroundColor Cyan
}

# ------------------------------------------------------------------
# パスの決定
# ------------------------------------------------------------------
$InstallerDir = $PSScriptRoot
$RootDir = Split-Path -Parent $InstallerDir
if ([string]::IsNullOrEmpty($SourceDir)) {
	$SourceDir = Join-Path $RootDir $Configuration
}
if ([string]::IsNullOrEmpty($OutDir)) {
	$OutDir = Join-Path $InstallerDir "out"
}
$WorkDir = Join-Path $InstallerDir "obj"
$StageDir = Join-Path $WorkDir "files"

# ------------------------------------------------------------------
# バージョンの取得
# ------------------------------------------------------------------
if ([string]::IsNullOrEmpty($Version)) {
	$rcPath = Join-Path $RootDir "CLCL.rc"
	if (-not (Test-Path $rcPath)) {
		throw "CLCL.rc が見つかりません: $rcPath"
	}
	$rcText = Get-Content -Path $rcPath -Raw -Encoding UTF8
	$m = [regex]::Match($rcText, 'FILEVERSION\s+(\d+)\s*,\s*(\d+)\s*,\s*(\d+)')
	if (-not $m.Success) {
		throw "CLCL.rc から FILEVERSION を取得できませんでした。"
	}
	$Version = "{0}.{1}.{2}" -f $m.Groups[1].Value, $m.Groups[2].Value, $m.Groups[3].Value
}
$vm = [regex]::Match($Version, '^(\d+)\.(\d+)\.(\d+)$')
if (-not $vm.Success) {
	throw "バージョンの指定が正しくありません (例: 2.2.0): $Version"
}
$VerMajor = [int]$vm.Groups[1].Value
$VerMinor = [int]$vm.Groups[2].Value
$VerRevision = [int]$vm.Groups[3].Value
$OutName = "clcl$VerMajor$VerMinor$VerRevision.exe"

Write-Step "CLCL Ver $Version のインストーラを作成します ($OutName)"

# ------------------------------------------------------------------
# Visual Studio の開発環境を読み込む
# ------------------------------------------------------------------
Write-Step "Visual Studio の開発環境を読み込みます"
$vsWhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vsWhere)) {
	throw "vswhere.exe が見つかりません。Visual Studio 2017 以降が必要です。"
}
$vsPath = & $vsWhere -latest -products * `
	-requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
	-property installationPath
if ([string]::IsNullOrEmpty($vsPath)) {
	throw "C++ のビルドツールを含む Visual Studio が見つかりません。"
}
$devCmd = Join-Path $vsPath "Common7\Tools\VsDevCmd.bat"
if (-not (Test-Path $devCmd)) {
	throw "VsDevCmd.bat が見つかりません: $devCmd"
}
$hostArch = if ([Environment]::Is64BitOperatingSystem) { "amd64" } else { "x86" }
cmd /c "`"$devCmd`" -arch=x86 -host_arch=$hostArch -no_logo && set" | ForEach-Object {
	if ($_ -match '^([^=]+)=(.*)$') {
		Set-Item -Path ("Env:\" + $matches[1]) -Value $matches[2]
	}
}
foreach ($tool in @("cl.exe", "rc.exe", "link.exe")) {
	if ($null -eq (Get-Command $tool -ErrorAction SilentlyContinue)) {
		throw "$tool が見つかりません。Windows SDK がインストールされているか確認してください。"
	}
}

# ------------------------------------------------------------------
# CLCL 本体のビルド
# ------------------------------------------------------------------
if (-not $SkipBuild) {
	Write-Step "CLCL 本体をビルドします ($Configuration|$Platform)"
	$slnPath = Join-Path $RootDir "CLCL.sln"
	& msbuild $slnPath /nologo /v:minimal /m "/p:Configuration=$Configuration" "/p:Platform=$Platform"
	if ($LASTEXITCODE -ne 0) {
		throw "CLCL 本体のビルドに失敗しました。"
	}
}

# ------------------------------------------------------------------
# インストールするファイルの収集
# ------------------------------------------------------------------
Write-Step "インストールするファイルを収集します"
if (Test-Path $WorkDir) {
	Remove-Item -Path $WorkDir -Recurse -Force
}
New-Item -Path $StageDir -ItemType Directory -Force | Out-Null

# 収集するファイル (収集元のパス = インストール後のファイル名)
$targetFiles = [ordered]@{
	(Join-Path $SourceDir "CLCL.exe")        = "CLCL.exe"
	(Join-Path $SourceDir "CLCLSet.exe")     = "CLCLSet.exe"
	(Join-Path $SourceDir "CLCLHook.dll")    = "CLCLHook.dll"
	(Join-Path $RootDir "readme_jp.txt")     = "readme_jp.txt"
	(Join-Path $RootDir "readme_en.txt")     = "readme_en.txt"
	(Join-Path $RootDir "readme_de.txt")     = "readme_de.txt"
	(Join-Path $RootDir "readme_uk.txt")     = "readme_uk.txt"
	(Join-Path $RootDir "readme_zh.txt")     = "readme_zh.txt"
	(Join-Path $RootDir "LICENSE")           = "LICENSE.txt"
}
foreach ($from in $targetFiles.Keys) {
	if (-not (Test-Path $from)) {
		throw "ファイルが見つかりません: $from"
	}
	Copy-Item -Path $from -Destination (Join-Path $StageDir $targetFiles[$from]) -Force
	Write-Host ("    {0}" -f $targetFiles[$from])
}

# ------------------------------------------------------------------
# ZIP の作成
# ------------------------------------------------------------------
Write-Step "インストールするファイルを ZIP にまとめます"
$zipPath = Join-Path $WorkDir "payload.zip"
Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory(
	$StageDir, $zipPath, [System.IO.Compression.CompressionLevel]::Optimal, $false)
Write-Host ("    payload.zip ({0:N0} bytes)" -f (Get-Item $zipPath).Length)

# ------------------------------------------------------------------
# バージョン情報のヘッダを作成
# ------------------------------------------------------------------
$instInfo = @"
/*
 * CLCL Installer
 *
 * instinfo.h
 *
 * build.ps1 が生成するファイルです。直接編集しないでください。
 */

#ifndef _INC_CLCL_INSTINFO_H
#define _INC_CLCL_INSTINFO_H

#define INST_VER_MAJOR					$VerMajor
#define INST_VER_MINOR					$VerMinor
#define INST_VER_REVISION				$VerRevision
#define INST_VERSION_STR				"$Version"
#define INST_FILE_VERSION_STR			"$VerMajor.$VerMinor.$VerRevision.0"
#define INST_FILE_NAME					"$OutName"

#endif
/* End of source */
"@
$instInfoPath = Join-Path $WorkDir "instinfo.h"
[System.IO.File]::WriteAllText($instInfoPath, $instInfo, (New-Object System.Text.UTF8Encoding $true))

# ------------------------------------------------------------------
# インストーラのコンパイル
# ------------------------------------------------------------------
Write-Step "インストーラをコンパイルします"
$resPath = Join-Path $WorkDir "Installer.res"
& rc.exe /nologo /i "$WorkDir" /i "$InstallerDir" /fo "$resPath" (Join-Path $InstallerDir "Installer.rc")
if ($LASTEXITCODE -ne 0) {
	throw "リソースのコンパイルに失敗しました。"
}

$clArgs = @(
	"/nologo", "/c", "/O2", "/MT", "/W3", "/GS-",
	"/DWIN32", "/D_WINDOWS", "/DNDEBUG", "/DUNICODE", "/D_UNICODE",
	"/D_WIN32_WINNT=0x0601", "/D_CRT_SECURE_NO_WARNINGS",
	"/I", "$WorkDir",
	"/Fo:$WorkDir\",
	(Join-Path $InstallerDir "Installer.c"),
	(Join-Path $InstallerDir "unzip.c")
)
& cl.exe @clArgs
if ($LASTEXITCODE -ne 0) {
	throw "インストーラのコンパイルに失敗しました。"
}

New-Item -Path $OutDir -ItemType Directory -Force | Out-Null
$outPath = Join-Path $OutDir $OutName
$linkArgs = @(
	"/nologo", "/SUBSYSTEM:WINDOWS", "/MANIFEST:NO", "/OPT:REF", "/OPT:ICF",
	"/OUT:$outPath",
	(Join-Path $WorkDir "Installer.obj"),
	(Join-Path $WorkDir "unzip.obj"),
	$resPath,
	"kernel32.lib", "user32.lib", "gdi32.lib", "advapi32.lib",
	"shell32.lib", "ole32.lib", "oleaut32.lib", "comctl32.lib", "uuid.lib"
)
& link.exe @linkArgs
if ($LASTEXITCODE -ne 0) {
	throw "インストーラのリンクに失敗しました。"
}

if (-not $KeepWork) {
	Remove-Item -Path $WorkDir -Recurse -Force
}

Write-Step "インストーラを作成しました"
Write-Host ("    {0} ({1:N0} bytes)" -f $outPath, (Get-Item $outPath).Length) -ForegroundColor Green

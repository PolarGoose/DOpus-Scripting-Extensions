#Requires -RunAsAdministrator

Function Info($msg) {
  Write-Host -ForegroundColor DarkGreen "`nINFO: $msg`n"
}

Function Error($msg) {
  Write-Host `n`n
  Write-Error $msg
  exit 1
}

Function CheckReturnCodeOfPreviousCommand($msg) {
  if(-Not $?) {
    Error "${msg}. Error code: $LastExitCode"
  }
}

Function FindMsBuild() {
  $vswhereCommand = Get-Command -Name "${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

  $msbuild = `
    & $vswhereCommand `
      -latest `
      -requires Microsoft.Component.MSBuild `
      -find MSBuild\**\Bin\MSBuild.exe `
      | select-object -first 1

  if(!$msbuild)
  {
    Error "Can't find MsBuild"
  }

  Info "MsBuild found: `n $msbuild"
  return $msbuild
}

Function FindVcPkg() {
  $vswhereCommand = Get-Command -Name "${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
  $installationPath = & $vswhereCommand -prerelease -latest -property installationPath
  return "$installationPath/VC/vcpkg/vcpkg.exe"
}

Function GetInstallerVersion() {
  $gitCommand = Get-Command -Name git

  try { $tag = & $gitCommand describe --exact-match --tags HEAD 2> $null } catch { }
  if(-Not $?) {
    Info "The commit is not tagged. Use 'v0.0' as a tag instead"
    $tag = "v0.0"
  }

  return $tag.Substring(1)
}

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$root = Resolve-Path $PSScriptRoot
$buildDir = "$root/build"
$installerVersion = GetInstallerVersion
$msbuild = FindMsBuild
$vcPkg = FindVcPkg

Info "InstallerVersion: '$installerVersion'"

Info "Integrate VcPkg"
& $vcPkg integrate install

Info "Build project"
& $msbuild `
    /nologo `
    /restore `
    /verbosity:minimal `
    /property:Configuration=Release `
    /property:DebugType=None `
    /property:InstallerVersion=$installerVersion `
    $root/DOpusScriptingExtensions.sln
CheckReturnCodeOfPreviousCommand "build failed"

Info "Run tests"
cscript $root/src/test/test.js
CheckReturnCodeOfPreviousCommand "tests failed"

Info "Copy installer to the Publish directory and create zip archive"
New-Item -Force -ItemType "directory" $buildDir/Publish > $null
Copy-Item -Force -Path $buildDir/x64/Release/Installer/Installer.msi -Destination $buildDir/Publish/DOpusScriptingExtensions.msi > $null
Compress-Archive -Force -Path $buildDir/Publish/DOpusScriptingExtensions.msi -DestinationPath $buildDir/Publish/DOpusScriptingExtensions.msi.zip

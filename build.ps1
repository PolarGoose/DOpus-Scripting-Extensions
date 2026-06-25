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
$buildDir = "$root/build/x64-Release"
$vcpkgInstallDir = "$root/build/vi"
$version = GetInstallerVersion

Info "InstallerVersion: '$version'"

Info "Find Visual Studio installation path"
$vswhereCommand = Get-Command -Name "${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$installationPath = & $vswhereCommand -prerelease -latest -property installationPath

Info "Open Visual Studio 2022 Developer PowerShell"
& "$installationPath\Common7\Tools\Launch-VsDevShell.ps1" -SkipAutomaticLocation -Arch amd64

Info "Cmake generate cache"
cmake `
  -S $root `
  -B $buildDir `
  -G Ninja `
  -D CMAKE_BUILD_TYPE=Release `
  -D CMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -D VCPKG_TARGET_TRIPLET=x64-windows-static `
  -D VCPKG_INSTALLED_DIR="$vcpkgInstallDir" `
  -D INSTALLER_VERSION=$version
CheckReturnCodeOfPreviousCommand "cmake cache failed"

Info "Cmake build"
cmake --build $buildDir
CheckReturnCodeOfPreviousCommand "cmake build failed"

Info "Uninstall the existing 'DOpusScriptingExtensions'"
Info "Search if 'DOpusScriptingExtensions' is installed"
$app = Get-CimInstance Win32_Product -Filter "Name = 'DOpusScriptingExtensions'"
if ($app) {
  Info "Uninstall existing 'DOpusScriptingExtensions' installation"
  Invoke-CimMethod -InputObject $app -MethodName Uninstall
}

$installerPath = Resolve-Path "$buildDir/Installer.msi"
Info "Install $installerPath"
$msiexecProcess = Start-Process -FilePath "msiexec.exe" `
                                -ArgumentList @("/i", $installerPath,
                                                "/norestart",
                                                "/L*v", "$buildDir/Installer.log") `
                                -Wait `
                                -PassThru
if ($msiexecProcess.ExitCode -ne 0) {
  Error "installation failed"
}

Info "Run tests"
cscript "$root/src/Test/test.js"
CheckReturnCodeOfPreviousCommand "tests failed"

Info "Create installer zip archive"
Copy-Item -Force -Path $buildDir/Installer.msi -Destination $buildDir/DOpusScriptingExtensions.msi > $null
Compress-Archive -Force -Path $buildDir/DOpusScriptingExtensions.msi -DestinationPath $buildDir/DOpusScriptingExtensions.msi.zip

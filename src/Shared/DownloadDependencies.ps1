param([Parameter(Mandatory = $true)] [string] $BuildDir,
      [Parameter(Mandatory = $true)] [string] $OutputDir)

Write-Host "Run script '$PSCommandPath'"

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$BuildDir = Resolve-Path $BuildDir
$OutputDir = Resolve-Path $OutputDir

Write-Host "BuildDir='$BuildDir'"
Write-Host "OutputDir='$OutputDir'"

Function CheckReturnCodeOfPreviousCommand($msg) {
  if(-Not $?) {
    Write-Error "${msg}. Error code: $LastExitCode"
  }
}

Function DownloadFile($Uri, $OutFile) {
    if (Test-Path $OutFile) {
        return
    }
    Write-Host "Download '$Uri' -> '$OutFile'"
    Invoke-WebRequest -Uri $Uri -OutFile $OutFile
}

Function DownloadZipAndUnpack($Uri, $OutFile, $OutDir) {
    if (Test-Path $OutDir) {
        return
    }
    DownloadFile -Uri $Uri -OutFile $OutFile
    New-Item -ItemType Directory `
             -Path $OutDir `
             -Force > $null
    Write-Host "Extract '$OutFile' -> '$OutDir'"
    Expand-Archive -Path $OutFile -DestinationPath $OutDir -Force
}

Write-Host "Download PROCEXP152.SYS file"

# There is no direct link to download PROCEXP152.SYS driver. However, the Handle tool contains it as a resource.
DownloadZipAndUnpack -Uri https://www.nirsoft.net/utils/resourcesextract-x64.zip `
                     -OutFile $BuildDir/resourcesextract-x64.zip `
                     -OutDir $BuildDir/resourcesextract
DownloadZipAndUnpack -Uri https://download.sysinternals.com/files/Handle.zip `
                     -OutFile $BuildDir/Handle.zip `
                     -OutDir $BuildDir/Handle

# ResourcesExtract.exe only accepts path with "\" separator. Otherwise it fails silently
& $BuildDir/resourcesextract/ResourcesExtract.exe /Source $BuildDir\Handle\handle64.exe /DestFolder $BuildDir /ExtractBinary 1 /FileExistMode 1 /OpenDestFolder 0
CheckReturnCodeOfPreviousCommand "ResourcesExtract failed"

# ResourcesExtract.exe works in parallel. We need to wait fot it to finish
Start-Sleep -Seconds 2

Remove-Item -Path $BuildDir/PROCEXP152.SYS.x64 -Force -ErrorAction SilentlyContinue > $null
Rename-Item $BuildDir/handle64_103_BINRES.bin -NewName PROCEXP152.SYS.x64

Copy-Item -Path $BuildDir/PROCEXP152.SYS.x64 -Destination $OutputDir/PROCEXP152.SYS.x64 -Force

Write-Host "Copy magic.mgc file that is needed for LibMagic"
Copy-Item -Path $BuildDir/vcpkg_installed/vcpkg/pkgs/libmagic_x64-windows/tools/libmagic/share/misc/magic.mgc -Destination $OutputDir/magic.mgc -Force

Write-Host "Download ExifTool binaries"
DownloadZipAndUnpack -Uri https://netix.dl.sourceforge.net/project/exiftool/exiftool-13.50_64.zip?viasf=1 `
                     -OutFile $BuildDir/exiftool-13.50_64.zip `
                     -OutDir $BuildDir/exiftool
Copy-Item -Path $BuildDir/exiftool/exiftool-13.50_64 -Destination $OutputDir/exiftool -Force -Recurse
Rename-Item "$OutputDir/exiftool/exiftool(-k).exe" -NewName "exiftool.exe"

Write-Host "Download language files for MediaInfoLib"
DownloadZipAndUnpack -Uri https://github.com/MediaArea/MediaInfo/archive/refs/tags/v26.01.zip `
                     -OutFile $BuildDir/MediaInfo.zip `
                     -OutDir $BuildDir/MediaInfo
Copy-Item -Path $BuildDir/MediaInfo/MediaInfo-26.01/Source/Resource/Plugin/Language -Destination $OutputDir/MediaInfoLanguages -Force -Recurse

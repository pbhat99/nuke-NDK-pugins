<#
.SYNOPSIS
    Nuke Plugin Build & Environment Manager (Windows x64)
.DESCRIPTION
    Interactive CLI menu system for managing local python virtual environments (.venv),
    discovering installed Nuke versions, building C++ NDK plugins with Visual Studio / CMake,
    deploying to COMPILED/Nuke<ver>/, and performing safe intermediate file cleanup.
    All workspace paths are dynamically resolved relative to build.bat / this script.
.NOTES
    Hardware Target : Generic GPU / VRAM Accelerated (NVIDIA CUDA / OptiX) & Multi-Core CPU
    Platform        : Windows only (x64)
#>

[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$MenuChoice = "",

    [Parameter(Position = 1)]
    [string]$TargetNuke = "",

    [Parameter(Position = 2)]
    [string]$TargetPlugin = "",

    [Parameter(Position = 3)]
    [string]$WorkspaceRoot = ""
)

# Set strict execution preferences
$ErrorActionPreference = "Stop"

# Dynamically resolve workspace root relative to build.bat / this script (no hardcoded paths)
if ([string]::IsNullOrWhiteSpace($WorkspaceRoot)) {
    if (-not [string]::IsNullOrWhiteSpace($PSScriptRoot)) {
        $WorkspaceRoot = $PSScriptRoot
    } else {
        $WorkspaceRoot = (Get-Location).Path
    }
}
$WorkspaceRoot = (Get-Item -LiteralPath $WorkspaceRoot).FullName.TrimEnd('\', '/')
Set-Location -LiteralPath $WorkspaceRoot

$VenvDir          = Join-Path $WorkspaceRoot ".venv"
$CompiledRoot     = Join-Path $WorkspaceRoot "COMPILED"
$RequirementsFile = Join-Path $WorkspaceRoot "requirements.txt"
$BuildTmpRoot     = Join-Path $WorkspaceRoot "_build_tmp"

# ==============================================================================
# Helper: Find Companion Scripts in COMPILED Root or Source
# ==============================================================================

function Get-CompanionFile {
    param(
        [string]$ToolName,
        [string]$FileName,
        [string]$PreferredSource = ""
    )
    if ($PreferredSource -and (Test-Path $PreferredSource)) {
        return $PreferredSource
    }
    # Look across COMPILED version folders for reference file
    if (Test-Path $CompiledRoot) {
        $found = Get-ChildItem -Path $CompiledRoot -Recurse -Filter $FileName -ErrorAction SilentlyContinue |
            Where-Object { $_.DirectoryName -match $ToolName } |
            Select-Object -First 1
        if ($found) { return $found.FullName }
    }
    return $null
}

# ==============================================================================
# Plugin Definitions Registry (13 Plugins)
# ==============================================================================
$PluginRegistry = @(
    @{
        Id          = "cropMore"
        Name        = "CropMore"
        Description = "Enhanced Nuke Crop node with auto-bounds & multi-frame autoCrop"
        SourceDir   = "cropMore"
        TargetDir   = "cropMore"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            $dll = Get-ChildItem $bDir -Recurse -Filter "CropMore.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            if (Test-Path "$sDir/init.py") { Copy-Item "$sDir/init.py" $dDir -Force }
            if (Test-Path "$sDir/menu.py") { Copy-Item "$sDir/menu.py" $dDir -Force }
        }
    },
    @{
        Id          = "AlphaToRoto"
        Name        = "AlphaToRoto"
        Description = "Trace alpha-channel mattes into editable Nuke Roto shapes"
        SourceDir   = "AlphaToRoto"
        TargetDir   = "AlphaToRoto"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            $dll = Get-ChildItem $bDir -Recurse -Filter "AlphaToRoto.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            $pySrc = if (Test-Path "$sDir/Python/alphaToRoto.py") {
                "$sDir/Python/alphaToRoto.py"
            } else {
                Get-CompanionFile -ToolName "AlphaToRoto" -FileName "alphaToRoto.py"
            }
            if ($pySrc) { Copy-Item $pySrc "$dDir/alphaToRoto.py" -Force }
            $menuSrc = Get-CompanionFile -ToolName "AlphaToRoto" -FileName "menu.py"
            if ($menuSrc) { Copy-Item $menuSrc $dDir -Force }
        }
    },
    @{
        Id          = "ColorGrade"
        Name        = "ColorGrade"
        Description = "Production-grade RGB color correction & grading operator"
        SourceDir   = "colorgrade"
        TargetDir   = "ColorGrade"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            $dll = Get-ChildItem $bDir -Recurse -Filter "ColorGrade.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            if (Test-Path "$sDir/python/init.py") { Copy-Item "$sDir/python/init.py" $dDir -Force }
            if (Test-Path "$sDir/python/menu.py") { Copy-Item "$sDir/python/menu.py" $dDir -Force }
        }
    },
    @{
        Id          = "DeepC"
        Name        = "DeepC"
        Description = "Comprehensive deep compositing toolset (20+ deep operators)"
        SourceDir   = "DeepC"
        TargetDir   = "DeepC"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            Get-ChildItem $bDir -Recurse -Filter "DeepC*.dll" | ForEach-Object {
                Copy-Item $_.FullName $dDir -Force
            }
            $thinner = Get-ChildItem $bDir -Recurse -Filter "DeepThinner.dll" | Select-Object -First 1
            if ($thinner) { Copy-Item $thinner.FullName $dDir -Force }
            if (Test-Path "$sDir/python/init.py") { Copy-Item "$sDir/python/init.py" $dDir -Force }
            $menuSrc = Get-CompanionFile -ToolName "DeepC" -FileName "menu.py"
            if ($menuSrc) { Copy-Item $menuSrc $dDir -Force }
            if (Test-Path "$sDir/icons") {
                $iconDest = Join-Path $dDir "icons"
                if (-not (Test-Path $iconDest)) { New-Item -ItemType Directory -Path $iconDest -Force | Out-Null }
                Copy-Item "$sDir/icons/*" $iconDest -Force
            }
        }
    },
    @{
        Id          = "DeepErode"
        Name        = "DeepErode"
        Description = "Deep morphological filtering and edge expansion/erosion"
        SourceDir   = "DeepErode"
        TargetDir   = "DeepErode"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            $dll = Get-ChildItem $bDir -Recurse -Filter "DeepErode.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            if (Test-Path "$sDir/python/init.py") { Copy-Item "$sDir/python/init.py" $dDir -Force }
            if (Test-Path "$sDir/python/menu.py") { Copy-Item "$sDir/python/menu.py" $dDir -Force }
        }
    },
    @{
        Id          = "FastNoiseLite"
        Name        = "FastNoiseLite"
        Description = "High-performance procedural coherent noise generator"
        SourceDir   = "FastNoiseLite"
        TargetDir   = "FastNoiseLite"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            $dll = Get-ChildItem $bDir -Recurse -Filter "FastNoiseLite.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            if (Test-Path "$sDir/python/init.py") { Copy-Item "$sDir/python/init.py" $dDir -Force }
            if (Test-Path "$sDir/python/menu.py") { Copy-Item "$sDir/python/menu.py" $dDir -Force }
        }
    },
    @{
        Id          = "ldpk"
        Name        = "ldpk"
        Description = "3DEqualizer Lens Distortion Plugin Kit (18 optical models)"
        SourceDir   = "ldpk"
        TargetDir   = "ldpk"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            Get-ChildItem $bDir -Recurse -Filter "LD_*.dll" | ForEach-Object {
                Copy-Item $_.FullName $dDir -Force
            }
            if (Test-Path "$sDir/cmake/init.py") { Copy-Item "$sDir/cmake/init.py" $dDir -Force }
            if (Test-Path "$sDir/cmake/menu.py") { Copy-Item "$sDir/cmake/menu.py" $dDir -Force }
        }
    },
    @{
        Id          = "NukeShenanigans"
        Name        = "NukeShenanigans"
        Description = "RemoveUnlimited wildcard remover, DeepKeymix, DeepVolumise, etc."
        SourceDir   = "NukeShenanigans"
        TargetDir   = "NukeShenanigans"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            Get-ChildItem $bDir -Recurse -Filter "*.dll" | ForEach-Object {
                Copy-Item $_.FullName $dDir -Force
            }
            $menuSrc = if (Test-Path "$sDir/menu.py") { "$sDir/menu.py" } else { Get-CompanionFile -ToolName "NukeShenanigans" -FileName "menu.py" }
            if ($menuSrc) { Copy-Item $menuSrc $dDir -Force }
        }
    },
    @{
        Id          = "oidnDenoise"
        Name        = "oidnDenoise"
        Description = "Intel Open Image Denoise (OIDN) AI denoiser"
        SourceDir   = "ODIN Denoiser"
        TargetDir   = "oidnDenoise"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            $dll = Get-ChildItem $bDir -Recurse -Filter "oidnDenoise.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            $oidnBin = "$sDir/dependencies/oidn-2.2.2/bin"
            if (Test-Path $oidnBin) {
                Get-ChildItem $oidnBin -Filter "*.dll" | ForEach-Object {
                    Copy-Item $_.FullName $dDir -Force
                }
            }
            if (Test-Path "$sDir/scripts/init.py") { Copy-Item "$sDir/scripts/init.py" $dDir -Force }
            if (Test-Path "$sDir/scripts/menu.py") { Copy-Item "$sDir/scripts/menu.py" $dDir -Force }
            if (Test-Path "$sDir/images/icon.png") { Copy-Item "$sDir/images/icon.png" $dDir -Force }
        }
    },
    @{
        Id          = "OptixDenoiser"
        Name        = "OptixDenoiser"
        Description = "NVIDIA OptiX AI denoiser hardware accelerated with generic GPU / VRAM scaling"
        SourceDir   = "OptixDenoiser"
        TargetDir   = "OptixDenoiser"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            $dll = Get-ChildItem $bDir -Recurse -Filter "OptiXDenoiser.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            if (Test-Path "$sDir/cmake/init.py") { Copy-Item "$sDir/cmake/init.py" $dDir -Force }
            if (Test-Path "$sDir/cmake/menu.py") { Copy-Item "$sDir/cmake/menu.py" $dDir -Force }
        }
    },
    @{
        Id          = "RotoReducer"
        Name        = "RotoReducer"
        Description = "Curve simplifier for Nuke Roto shapes & native C++ accelerator"
        SourceDir   = "rotoReducer"
        TargetDir   = "RotoReducer"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            Get-ChildItem $bDir -Recurse -Filter "*.dll" | ForEach-Object {
                Copy-Item $_.FullName $dDir -Force
            }
            Get-ChildItem $bDir -Recurse -Filter "*.pyd" | ForEach-Object {
                Copy-Item $_.FullName $dDir -Force
            }
            if (Test-Path "$sDir/roto_reducer.py") { Copy-Item "$sDir/roto_reducer.py" $dDir -Force }
            if (Test-Path "$sDir/menu.py") { Copy-Item "$sDir/menu.py" $dDir -Force }
        }
    },
    @{
        Id          = "USDsubD"
        Name        = "USDsubD"
        Description = "OpenSubdiv Catmull-Clark and Loop mesh subdivision for USD"
        SourceDir   = "USDsubD"
        TargetDir   = "USDsubD"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            $dll = Get-ChildItem $bDir -Recurse -Filter "USDSubD.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            if (Test-Path "$sDir/menu.py") { Copy-Item "$sDir/menu.py" $dDir -Force }
        }
    },
    @{
        Id          = "bamDeVe"
        Name        = "bamDeVe"
        Description = "Estimate lens vignette from motion (Native C++ NDK, pure C++, zero NumPy)"
        SourceDir   = "bamDeVe"
        TargetDir   = "bamDeVe"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            if (-not (Test-Path $dDir)) {
                New-Item -ItemType Directory -Path $dDir -Force | Out-Null
            }
            $dll = Get-ChildItem $bDir -Recurse -Filter "bamDeVe.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            if (Test-Path "$sDir/bamDeVe_solve.py") { Copy-Item "$sDir/bamDeVe_solve.py" $dDir -Force }
            if (Test-Path "$sDir/init.py") { Copy-Item "$sDir/init.py" $dDir -Force }
            if (Test-Path "$sDir/menu.py") { Copy-Item "$sDir/menu.py" $dDir -Force }
        }
    },
    @{
        Id          = "ImageTool"
        Name        = "ImageTool"
        Description = "Live image statistics (Average, Median, Low, High) solid color generator"
        SourceDir   = "ImageTool"
        TargetDir   = "ImageTool"
        PostBuild   = {
            param($bDir, $dDir, $sDir)
            if (-not (Test-Path $dDir)) {
                New-Item -ItemType Directory -Path $dDir -Force | Out-Null
            }
            $dll = Get-ChildItem $bDir -Recurse -Filter "ImageTool.dll" | Select-Object -First 1
            if ($dll) { Copy-Item $dll.FullName $dDir -Force }
            if (Test-Path "$sDir/init.py") { Copy-Item "$sDir/init.py" $dDir -Force }
            if (Test-Path "$sDir/menu.py") { Copy-Item "$sDir/menu.py" $dDir -Force }
        }
    }
)

# ==============================================================================
# Helper: Environment & Tool Detection
# ==============================================================================

function Get-PythonHost {
    # 1. Existing .venv python
    $venvPy = Join-Path $VenvDir "Scripts\python.exe"
    if (Test-Path $venvPy) {
        return @{ Path = $venvPy; IsVenv = $true }
    }
    # 2. System PATH python
    $pyCmd = Get-Command python.exe -ErrorAction SilentlyContinue
    if ($pyCmd) {
        return @{ Path = $pyCmd.Source; IsVenv = $false }
    }
    # 3. py.exe launcher
    $pyLauncher = Get-Command py.exe -ErrorAction SilentlyContinue
    if ($pyLauncher) {
        return @{ Path = $pyLauncher.Source; IsVenv = $false }
    }
    # 4. Standard user AppData locations
    $userPy = Get-ChildItem "$env:LOCALAPPDATA\Programs\Python" -Filter "python.exe" -Recurse -Depth 2 -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($userPy) {
        return @{ Path = $userPy.FullName; IsVenv = $false }
    }
    return $null
}

function Get-CMakeExecutable {
    # 1. System PATH
    $cmakeCmd = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($cmakeCmd) { return $cmakeCmd.Source }
    # 2. .venv Scripts
    $venvCmake = Join-Path $VenvDir "Scripts\cmake.exe"
    if (Test-Path $venvCmake) { return $venvCmake }
    # 3. Program Files standard install
    $pfCmake = "C:\Program Files\CMake\bin\cmake.exe"
    if (Test-Path $pfCmake) { return $pfCmake }
    # 4. Visual Studio embedded CMake
    $vsPath = Get-VisualStudioPath
    if ($vsPath) {
        $vsCmake = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        if (Test-Path $vsCmake) { return $vsCmake }
    }
    return $null
}

function Get-VisualStudioPath {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsInst = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsInst -and (Test-Path $vsInst)) {
            return $vsInst
        }
    }
    # Fallback paths
    $stdPaths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise",
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
    )
    foreach ($p in $stdPaths) {
        if (Test-Path $p) { return $p }
    }
    return $null
}

function Get-InstalledNukeVersions {
    $nukeList = [System.Collections.Generic.List[PSCustomObject]]::new()
    $searchDirs = @("C:\Program Files", "C:\Program Files\Foundry")

    foreach ($searchDir in $searchDirs) {
        if (-not (Test-Path $searchDir)) { continue }
        $candidates = Get-ChildItem -Directory $searchDir -Filter "Nuke*" -ErrorAction SilentlyContinue
        foreach ($c in $candidates) {
            $dir = $c.FullName
            $ddLib = Join-Path $dir "DDImage.lib"
            $header = Join-Path $dir "include\DDImage\ddImageVersionNumbers.h"

            if (Test-Path $ddLib) {
                $verString = ""
                $folderTag = ""
                $major = 0
                $minor = 0

                if (Test-Path $header) {
                    $content = Get-Content $header -Raw -ErrorAction SilentlyContinue
                    $major = [regex]::Match($content, 'kDDImageVersionMajorNum\s+(\d+)').Groups[1].Value
                    $minor = [regex]::Match($content, 'kDDImageVersionMinorNum\s+(\d+)').Groups[1].Value
                    $rel = [regex]::Match($content, 'kDDImageVersionReleaseNum\s+(\d+)').Groups[1].Value
                    $verString = "${major}.${minor}v${rel}"
                    $folderTag = "Nuke${major}.${minor}"
                } else {
                    if ($c.Name -match 'Nuke(\d+)\.(\d+)(?:v(\d+))?') {
                        $major = $Matches[1]
                        $minor = $Matches[2]
                        $rel = if ($Matches[3]) { $Matches[3] } else { "1" }
                        $verString = "${major}.${minor}v${rel}"
                        $folderTag = "Nuke${major}.${minor}"
                    } else {
                        $folderTag = $c.Name
                        $verString = $c.Name.Replace("Nuke", "")
                    }
                }

                $nukeList.Add([PSCustomObject]@{
                    Name      = "Nuke $verString"
                    Version   = $verString
                    FolderTag = $folderTag
                    Path      = $dir
                    Major     = [int]$major
                    Minor     = [int]$minor
                })
            }
        }
    }
    return $nukeList | Sort-Object Major, Minor -Descending
}

# ==============================================================================
# Helper: Guides & Links for External Toolchain (CMake, MSVC, Python, Nuke)
# ==============================================================================

function Show-ToolchainGuide {
    Safe-ClearHost
    Write-Host "========================================================================" -ForegroundColor Magenta
    Write-Host "            EXTERNAL TOOLCHAIN INSTALLATION GUIDES & LINKS              " -ForegroundColor Yellow
    Write-Host "========================================================================" -ForegroundColor Magenta
    Write-Host "Compiling C++ NDK plugins requires external build tools." -ForegroundColor Gray
    Write-Host "Some tools require administrative privileges to install into Windows." -ForegroundColor Gray
    Write-Host "------------------------------------------------------------------------" -ForegroundColor DarkGray
    Write-Host ""

    # 1. Visual Studio / MSVC Guide
    Write-Host "[1] Visual Studio 2022 C++ Compiler (MSVC v143) & Windows SDK" -ForegroundColor Cyan
    Write-Host "    Purpose     : C++ compiler and linker for native .dll / .pyd plugins" -ForegroundColor Gray
    Write-Host "    Admin Rights: REQUIRED (Requires admin to install compilers into system)" -ForegroundColor Yellow
    Write-Host "    Download Links:" -ForegroundColor White
    Write-Host "      - VS Build Tools 2022 (CLI only, lightweight):" -ForegroundColor White
    Write-Host "        https://aka.ms/vs/17/release/vs_BuildTools.exe" -ForegroundColor Green
    Write-Host "      - Visual Studio Community 2022 (Full IDE, Free):" -ForegroundColor White
    Write-Host "        https://visualstudio.microsoft.com/downloads/" -ForegroundColor Green
    Write-Host "    One-Command Install (Run in PowerShell as Administrator):" -ForegroundColor White
    Write-Host "      winget install Microsoft.VisualStudio.2022.BuildTools --override `"--passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended`"" -ForegroundColor DarkCyan
    Write-Host "    GUI Installation Checklist:" -ForegroundColor White
    Write-Host "      1. In Visual Studio Installer, select: 'Desktop development with C++'" -ForegroundColor Gray
    Write-Host "      2. In the right panel, verify these items are checked:" -ForegroundColor Gray
    Write-Host "         [x] MSVC v143 - VS 2022 C++ x64/x86 build tools" -ForegroundColor Gray
    Write-Host "         [x] Windows 10 SDK or Windows 11 SDK" -ForegroundColor Gray
    Write-Host "         [x] C++ CMake tools for Windows" -ForegroundColor Gray
    Write-Host ""

    # 2. CMake Guide
    Write-Host "[2] CMake (Build Automation System, Version 3.18+)" -ForegroundColor Cyan
    Write-Host "    Purpose     : Configures and generates Visual Studio solution files" -ForegroundColor Gray
    Write-Host "    Admin Rights: OPTIONAL" -ForegroundColor Yellow
    Write-Host "      * No Admin Needed: Menu [0] can auto-install CMake inside .venv (`pip install cmake`)" -ForegroundColor Green
    Write-Host "      * Portable ZIP   : Download .zip archive and extract anywhere (no admin)" -ForegroundColor Green
    Write-Host "      * System (.msi)  : Standard Windows installer (requires admin)" -ForegroundColor Gray
    Write-Host "    Download Links:" -ForegroundColor White
    Write-Host "      - Official Downloads (Installer & Portable ZIP):" -ForegroundColor White
    Write-Host "        https://cmake.org/download/" -ForegroundColor Green
    Write-Host "    One-Command Install via Winget (if admin):" -ForegroundColor White
    Write-Host "      winget install Kitware.CMake" -ForegroundColor DarkCyan
    Write-Host ""

    # 3. Python Guide
    Write-Host "[3] Python 3 (Python 3.10 - 3.14)" -ForegroundColor Cyan
    Write-Host "    Purpose     : Powers local .venv, pip package management, and helper scripts" -ForegroundColor Gray
    Write-Host "    Admin Rights: OPTIONAL" -ForegroundColor Yellow
    Write-Host "      * No Admin Needed: Microsoft Store Python installs per-user without admin!" -ForegroundColor Green
    Write-Host "    Download Links:" -ForegroundColor White
    Write-Host "      - Microsoft Store: Search for 'Python 3.11' or 'Python 3.12'" -ForegroundColor Green
    Write-Host "      - Official Installer (.exe): https://www.python.org/downloads/windows/" -ForegroundColor Green
    Write-Host "    One-Command Install via Winget:" -ForegroundColor White
    Write-Host "      winget install Python.Python.3.11" -ForegroundColor DarkCyan
    Write-Host ""

    # 4. Nuke NDK Guide
    Write-Host "[4] Foundry Nuke & NDK (Header Files & DDImage.lib)" -ForegroundColor Cyan
    Write-Host "    Purpose     : Provides DDImage headers and linking libraries" -ForegroundColor Gray
    Write-Host "    Download Link:" -ForegroundColor White
    Write-Host "      - Foundry Portal: https://www.foundry.com/products/nuke/download" -ForegroundColor Green
    Write-Host "    Default Search Paths:" -ForegroundColor White
    Write-Host "      - C:\Program Files\Nuke*" -ForegroundColor DarkCyan
    Write-Host "      - C:\Program Files\Foundry\Nuke*" -ForegroundColor DarkCyan
    Write-Host "    Custom Path: If installed on another drive, Menu [1] accepts custom directories." -ForegroundColor Gray
    Write-Host ""
    Write-Host "========================================================================" -ForegroundColor Magenta
    Write-Host ""
    Pause-Prompt "Press [Enter] to return to menu..."
}

# ==============================================================================
# Helper: Safe UI Navigation & Fallback
# ==============================================================================

function Pause-Prompt {
    param([string]$Message = "Press [Enter] to continue...")
    if (-not [Console]::IsInputRedirected) {
        Read-Host $Message | Out-Null
    }
}

function Safe-ClearHost {
    try {
        if (-not [Console]::IsOutputRedirected -and -not [Console]::IsInputRedirected) {
            Clear-Host
        }
    } catch {}
}

function Show-InteractiveMenu {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Title,

        [Parameter(Mandatory = $true)]
        [string[]]$Options,

        [int]$DefaultIndex = 0,
        [string]$SubTitle = "",
        [string]$Footer = "[Up/Down] Navigate  |  [Enter] Select  |  [0-9] Quick Jump  |  [Esc] Back/Exit"
    )

    $count = $Options.Count
    if ($count -eq 0) { return -1 }

    # Fallback for redirected shells / background runs
    if ([Console]::IsInputRedirected) {
        Write-Host ""
        Write-Host "========================================================================" -ForegroundColor Cyan
        Write-Host "  $Title" -ForegroundColor Yellow
        if ($SubTitle) { Write-Host "  $SubTitle" -ForegroundColor Gray }
        Write-Host "========================================================================" -ForegroundColor Cyan
        for ($i = 0; $i -lt $count; $i++) {
            Write-Host "  [$i] $($Options[$i])"
        }
        Write-Host "------------------------------------------------------------------------" -ForegroundColor DarkGray
        $choice = Read-Host "Select item [0-$($count - 1)]"
        if ($choice -match '^\d+$' -and [int]$choice -lt $count) {
            return [int]$choice
        }
        return -1
    }

    # Interactive Console mode with smooth Arrow Key navigation
    $origCursor = $true
    try { $origCursor = [Console]::CursorVisible; [Console]::CursorVisible = $false } catch {}

    $current = $DefaultIndex
    if ($current -lt 0 -or $current -ge $count) { $current = 0 }

    Safe-ClearHost

    while ($true) {
        try { [Console]::SetCursorPosition(0, 0) } catch {}

        Write-Host "========================================================================" -ForegroundColor Cyan
        Write-Host ("  {0,-68}" -f $Title) -ForegroundColor Yellow
        if ($SubTitle) {
            Write-Host ("  {0,-68}" -f $SubTitle) -ForegroundColor DarkCyan
        }
        Write-Host "========================================================================" -ForegroundColor Cyan

        for ($i = 0; $i -lt $count; $i++) {
            if ($i -eq $current) {
                Write-Host "  > " -NoNewline -ForegroundColor Green
                Write-Host ("[{0}] {1,-62}" -f $i, $Options[$i]) -ForegroundColor Black -BackgroundColor Green
            } else {
                Write-Host "    " -NoNewline
                Write-Host ("[{0}] {1,-62}" -f $i, $Options[$i]) -ForegroundColor White
            }
        }

        Write-Host "------------------------------------------------------------------------" -ForegroundColor DarkGray
        Write-Host ("  {0,-68}" -f $Footer) -ForegroundColor DarkGray

        $keyInfo = [Console]::ReadKey($true)

        switch ($keyInfo.Key) {
            'UpArrow' {
                $current = ($current - 1 + $count) % $count
            }
            'DownArrow' {
                $current = ($current + 1) % $count
            }
            'Home' {
                $current = 0
            }
            'End' {
                $current = $count - 1
            }
            'Enter' {
                try { [Console]::CursorVisible = $origCursor } catch {}
                Write-Host ""
                return $current
            }
            'Escape' {
                try { [Console]::CursorVisible = $origCursor } catch {}
                Write-Host ""
                return -1
            }
            default {
                $char = $keyInfo.KeyChar
                if ($char -ge '0' -and $char -le '9') {
                    $val = [int][string]$char
                    if ($val -lt $count) {
                        try { [Console]::CursorVisible = $origCursor } catch {}
                        Write-Host ""
                        return $val
                    }
                }
                if ($keyInfo.Key -eq 'Q' -or $keyInfo.Key -eq 'Escape') {
                    try { [Console]::CursorVisible = $origCursor } catch {}
                    Write-Host ""
                    return -1
                }
                if ($keyInfo.Key -eq 'G') {
                    try { [Console]::CursorVisible = $origCursor } catch {}
                    Write-Host ""
                    return 999
                }
            }
        }
    }
}

# ==============================================================================
# MENU 0: Install Build Requirements (.venv without admin & toolchain check)
# ==============================================================================

function Invoke-InstallRequirements {
    Safe-ClearHost
    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "  MENU 0: INSTALL BUILD REQUIREMENTS (.venv & TOOLCHAIN AUDIT)" -ForegroundColor Yellow
    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "Workspace Root : $WorkspaceRoot (relative to build.bat)" -ForegroundColor Gray
    Write-Host ""

    # 1. Locate host Python
    $pyHost = Get-PythonHost
    if (-not $pyHost) {
        Write-Host "[ERROR] Python 3 was not found on your system." -ForegroundColor Red
        Write-Host ""
        Write-Host "Installation Guide:" -ForegroundColor Yellow
        Write-Host "  * No admin rights needed: Install Python 3.11 / 3.12 from Microsoft Store" -ForegroundColor Green
        Write-Host "  * Installer download    : https://www.python.org/downloads/windows/" -ForegroundColor Green
        Write-Host "  * Winget command        : winget install Python.Python.3.11" -ForegroundColor DarkCyan
        Write-Host ""
        Pause-Prompt "Press [Enter] to return..."
        return
    }
    Write-Host "[INFO] Detected Python Host : $($pyHost.Path)" -ForegroundColor Green

    # 2. Check / Create .venv
    $venvPy  = Join-Path $VenvDir "Scripts\python.exe"
    $venvPip = Join-Path $VenvDir "Scripts\pip.exe"

    if (-not (Test-Path $venvPy)) {
        Write-Host "[INFO] Creating local virtual environment at: .venv ... " -NoNewline -ForegroundColor Cyan
        try {
            & "$($pyHost.Path)" -m venv "$VenvDir"
            Write-Host "DONE" -ForegroundColor Green
        } catch {
            Write-Host "FAILED" -ForegroundColor Red
            Write-Host "[ERROR] Could not create virtual environment: $_" -ForegroundColor Red
            Write-Host ""
            Pause-Prompt "Press [Enter] to return..."
            return
        }
    } else {
        Write-Host "[INFO] Local virtual environment already exists at: .venv" -ForegroundColor Green
    }

    # 3. Upgrade pip inside .venv
    Write-Host "[INFO] Upgrading pip inside .venv ... " -NoNewline -ForegroundColor Cyan
    try {
        & "$venvPy" -m pip install --upgrade pip --quiet --no-warn-script-location
        Write-Host "DONE" -ForegroundColor Green
    } catch {
        Write-Host "WARNING (pip upgrade skipped)" -ForegroundColor DarkYellow
    }

    # 4. Install requirements.txt
    if (Test-Path $RequirementsFile) {
        Write-Host "[INFO] Installing build dependencies from requirements.txt ... " -ForegroundColor Cyan
        try {
            & "$venvPip" install -r "$RequirementsFile" --no-warn-script-location
            Write-Host "[INFO] Python build dependencies successfully installed!" -ForegroundColor Green
        } catch {
            Write-Host "[ERROR] Failed to install requirements: $_" -ForegroundColor Red
        }
    }

    # 5. Check CMake & attempt automatic local install into .venv if missing
    $cmakeExe = Get-CMakeExecutable
    if (-not $cmakeExe) {
        Write-Host "[INFO] CMake not found on PATH. Attempting automatic install into .venv (no admin needed) ... " -NoNewline -ForegroundColor Cyan
        try {
            & "$venvPip" install cmake --quiet --no-warn-script-location
            $cmakeExe = Get-CMakeExecutable
            if ($cmakeExe) {
                Write-Host "DONE (CMake installed inside .venv!)" -ForegroundColor Green
            } else {
                Write-Host "FAILED" -ForegroundColor DarkYellow
            }
        } catch {
            Write-Host "FAILED" -ForegroundColor DarkYellow
        }
    }

    $cmakeStatus = if ($cmakeExe) {
        $cmakeVer = & $cmakeExe --version | Select-Object -First 1
        "[OK] $cmakeVer ($cmakeExe)"
    } else {
        "[MISSING] cmake.exe not found on system PATH or in .venv"
    }

    # 6. Check Visual Studio C++ Compiler
    $vsPath = Get-VisualStudioPath
    $vsStatus = if ($vsPath) {
        "[OK] Visual Studio detected at: $vsPath"
    } else {
        "[MISSING] Visual Studio 2022 C++ Build Tools (MSVC v143) not found"
    }

    # 7. Check Installed Nuke Versions
    $nukeVersions = Get-InstalledNukeVersions

    # 8. Report Summary Dashboard
    $cmakeColor = if ($cmakeExe) { "Green" } else { "Red" }
    $vsColor    = if ($vsPath) { "Green" } else { "Red" }

    Write-Host ""
    Write-Host "------------------------------------------------------------------------" -ForegroundColor Cyan
    Write-Host "                     BUILD ENVIRONMENT SUMMARY                          " -ForegroundColor Yellow
    Write-Host "------------------------------------------------------------------------" -ForegroundColor Cyan
    Write-Host "  Workspace    : " -NoNewline; Write-Host "$WorkspaceRoot" -ForegroundColor Green
    Write-Host "  Virtual Env  : " -NoNewline; Write-Host "$VenvDir" -ForegroundColor Green
    Write-Host "  Venv Python  : " -NoNewline; Write-Host "$venvPy" -ForegroundColor Green
    Write-Host "  CMake Tool   : " -NoNewline; Write-Host "$cmakeStatus" -ForegroundColor $cmakeColor
    Write-Host "  Compiler     : " -NoNewline; Write-Host "$vsStatus" -ForegroundColor $vsColor
    Write-Host "  Hardware     : " -NoNewline; Write-Host "Generic GPU (VRAM auto-scaling) / Multi-core CPU" -ForegroundColor Green
    Write-Host "  Nuke NDK(s)  : " -ForegroundColor Cyan

    if ($nukeVersions.Count -gt 0) {
        foreach ($nv in $nukeVersions) {
            Write-Host "    - $($nv.Name) [NDK ready: $($nv.Path)]" -ForegroundColor Green
        }
    } else {
        Write-Host "    [WARNING] No Nuke installations detected in C:\Program Files\Nuke*" -ForegroundColor Yellow
    }

    Write-Host "------------------------------------------------------------------------" -ForegroundColor Cyan

    # If any tool is missing, show concise guidance and link immediately
    if (-not $cmakeExe -or -not $vsPath) {
        Write-Host ""
        Write-Host "[ACTION REQUIRED] Missing prerequisites detected:" -ForegroundColor Yellow
        if (-not $cmakeExe) {
            Write-Host "  * CMake missing:" -ForegroundColor Red
            Write-Host "    - Download portable ZIP (no admin) : https://cmake.org/download/" -ForegroundColor Gray
            Write-Host "    - Or install via winget (if admin) : winget install Kitware.CMake" -ForegroundColor Gray
        }
        if (-not $vsPath) {
            Write-Host "  * Visual Studio C++ (MSVC) missing (requires admin):" -ForegroundColor Red
            Write-Host "    - Download VS Build Tools 2022     : https://aka.ms/vs/17/release/vs_BuildTools.exe" -ForegroundColor Gray
            Write-Host "    - In installer, select             : 'Desktop development with C++'" -ForegroundColor Gray
            Write-Host "    - Or run in Admin PowerShell       : winget install Microsoft.VisualStudio.2022.BuildTools --override `"--passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended`"" -ForegroundColor DarkCyan
        }
    }

    Write-Host ""
    Write-Host "Press [G] to view full Toolchain Installation Guide, or [Enter] to return..." -ForegroundColor Gray
    if (-not [Console]::IsInputRedirected) {
        $key = [Console]::ReadKey($true)
        if ($key.Key -eq 'G') {
            Show-ToolchainGuide
            return
        }
    }
}

# ==============================================================================
# MENU 1: Build Plugins (Dynamic Nuke & Plugin Submenus)
# ==============================================================================

function Build-SinglePlugin {
    param(
        [Parameter(Mandatory = $true)]
        $Plugin,

        [Parameter(Mandatory = $true)]
        $NukeTarget,

        [Parameter(Mandatory = $true)]
        [string]$CMakeExe
    )

    $pluginId   = $Plugin.Id
    $pluginName = $Plugin.Name
    $srcDir     = Join-Path $WorkspaceRoot $Plugin.SourceDir
    $folderTag  = $NukeTarget.FolderTag
    $deployDir  = Join-Path $CompiledRoot "WIN\$folderTag\$($Plugin.TargetDir)"
    $tmpBuildDir = Join-Path $BuildTmpRoot "${pluginId}_${folderTag}"

    Write-Host ""
    Write-Host ">>> Building [$pluginName] for $($NukeTarget.Name) ..." -ForegroundColor Cyan
    Write-Host "    Source : $srcDir" -ForegroundColor Gray
    Write-Host "    Deploy : $deployDir" -ForegroundColor Gray

    if (-not (Test-Path $deployDir)) {
        New-Item -ItemType Directory -Path $deployDir -Force | Out-Null
    }

    # Clean previous temp build dir if exists
    if (Test-Path $tmpBuildDir) {
        Remove-Item -Recurse -Force $tmpBuildDir -ErrorAction SilentlyContinue
    }

    $nukePathNorm = $NukeTarget.Path -replace '\\', '/'

    # Set scoped environment variables for Nuke discovery
    $env:NDKDIR            = $nukePathNorm
    $env:Nuke_ROOT         = $nukePathNorm
    $env:NUKE_INSTALL_PATH = $nukePathNorm
    $env:NUKE_INSTALL_DIR  = $nukePathNorm

    $cmakeConfigureArgs = @(
        "--no-warn-unused-cli",
        "-B", $tmpBuildDir,
        "-S", $srcDir,
        "-G", "Visual Studio 17 2022",
        "-A", "x64",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DNUKE_VERSION=$($NukeTarget.Version)",
        "-DNDKDIR=$nukePathNorm",
        "-DNuke_ROOT=$nukePathNorm",
        "-DNUKE_INSTALL_PATH=$nukePathNorm",
        "-DNUKE_INSTALL_DIR=$nukePathNorm"
    )

    # Dynamic plugin-specific arguments relative to workspace
    if ($pluginId -eq "oidnDenoise") {
        $oidnPath = (Join-Path $srcDir "dependencies/oidn-2.2.2") -replace '\\', '/'
        $cmakeConfigureArgs += "-DOIDN_ROOT=$oidnPath"
    } elseif ($pluginId -eq "DeepC") {
        $cmakeConfigureArgs += "-DDEEPC_BUILD_TESTS=OFF"
    }

    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

    if ($Plugin.IsPythonOnly) {
        Write-Host "    [1/1] Deploying Python / Gizmo plugin files ..." -ForegroundColor DarkCyan
        & $Plugin.PostBuild $tmpBuildDir $deployDir $srcDir
        $stopwatch.Stop()
        Write-Host "    [SUCCESS] $pluginName deployed in $($stopwatch.Elapsed.TotalSeconds.ToString('F2'))s!" -ForegroundColor Green
        return $true
    }

    $prevEAP = $ErrorActionPreference
    $hasNativePref = Test-Path Variable:\PSNativeCommandUseErrorActionPreference
    $prevNativePref = if ($hasNativePref) { $PSNativeCommandUseErrorActionPreference } else { $false }

    try {
        $ErrorActionPreference = "Continue"
        if ($hasNativePref) { $PSNativeCommandUseErrorActionPreference = $false }

        # 1. CMake Configure
        Write-Host "    [1/2] Configuring CMake ..." -ForegroundColor DarkCyan
        $cfgOutput = & $CMakeExe @cmakeConfigureArgs 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[ERROR] CMake configure failed for $pluginName`:" -ForegroundColor Red
            $cfgOutput | ForEach-Object { Write-Host "      $_" -ForegroundColor DarkRed }
            return $false
        }

        # 2. CMake Build
        Write-Host "    [2/2] Compiling with MSVC (/MP Release) ..." -ForegroundColor DarkCyan
        $buildOutput = & $CMakeExe --build $tmpBuildDir --config Release --parallel 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[ERROR] Compilation failed for $pluginName`:" -ForegroundColor Red
            $buildOutput | ForEach-Object { Write-Host "      $_" -ForegroundColor DarkRed }
            return $false
        }

        # 3. PostBuild Deployment
        & $Plugin.PostBuild $tmpBuildDir $deployDir $srcDir

        $stopwatch.Stop()
        Write-Host "    [SUCCESS] $pluginName built and deployed in $($stopwatch.Elapsed.TotalSeconds.ToString('F1'))s!" -ForegroundColor Green

        # Clean intermediate build dir to save disk space
        Remove-Item -Recurse -Force $tmpBuildDir -ErrorAction SilentlyContinue
        return $true
    } catch {
        Write-Host "[ERROR] Exception during $pluginName build: $_" -ForegroundColor Red
        return $false
    } finally {
        $ErrorActionPreference = $prevEAP
        if ($hasNativePref) { $PSNativeCommandUseErrorActionPreference = $prevNativePref }
    }
}

function Invoke-BuildPlugins {
    param(
        [string]$NukeFilter = "",
        [string]$PluginFilter = ""
    )

    $cmakeExe = Get-CMakeExecutable
    if (-not $cmakeExe) {
        Write-Host "[ERROR] CMake was not found. Please run [0] Install Build Requirements first." -ForegroundColor Red
        Pause-Prompt "Press [Enter] to return..."
        return
    }

    # Discover installed Nuke versions
    $nukeVersions = Get-InstalledNukeVersions

    if ($nukeVersions.Count -eq 0) {
        Write-Host "[WARNING] No Nuke installations were automatically found in C:\Program Files\Nuke*" -ForegroundColor Yellow
        $customPath = if (-not [Console]::IsInputRedirected) {
            Read-Host "Enter path to Nuke installation (e.g. C:\Program Files\Nuke17.1v2) or press [Enter] to cancel"
        } else { "" }

        if ($customPath -and (Test-Path $customPath)) {
            $ddLib = Join-Path $customPath "DDImage.lib"
            if (Test-Path $ddLib) {
                $nukeVersions = @(
                    [PSCustomObject]@{
                        Name      = "Custom Nuke ($customPath)"
                        Version   = "Custom"
                        FolderTag = "NukeCustom"
                        Path      = $customPath
                        Major     = 0
                        Minor     = 0
                    }
                )
            } else {
                Write-Host "[ERROR] DDImage.lib not found in $customPath" -ForegroundColor Red
                Pause-Prompt "Press [Enter] to return..."
                return
            }
        } else {
            return
        }
    }

    $targetsToBuild = @()

    # If Nuke filter was provided via parameter or CLI
    if ($NukeFilter -ne "") {
        if ($NukeFilter -ieq "ALL") {
            $targetsToBuild = $nukeVersions
        } else {
            $matched = $nukeVersions | Where-Object { $_.Version -match $NukeFilter -or $_.FolderTag -match $NukeFilter }
            if ($matched) {
                $targetsToBuild = @($matched)
            } else {
                Write-Host "[ERROR] No installed Nuke matches filter: '$NukeFilter'" -ForegroundColor Red
                Pause-Prompt "Press [Enter] to return..."
                return
            }
        }
    } else {
        # ----------------------------------------------------------------------
        # Submenu 1: Select Nuke Version
        # ----------------------------------------------------------------------
        $nukeMenuOptions = @()
        foreach ($nv in $nukeVersions) {
            $nukeMenuOptions += "$($nv.Name)  ($($nv.Path))"
        }
        $nukeMenuOptions += "Build for ALL Detected Nuke Versions"
        $nukeMenuOptions += "Return to Main Menu"

        $selectedNukeIdx = Show-InteractiveMenu `
            -Title "SELECT NUKE VERSION TARGET" `
            -SubTitle "Detected $($nukeVersions.Count) Nuke installation(s) with NDK headers" `
            -Options $nukeMenuOptions

        if ($selectedNukeIdx -lt 0 -or $selectedNukeIdx -eq ($nukeMenuOptions.Count - 1)) {
            return
        }

        if ($selectedNukeIdx -eq ($nukeMenuOptions.Count - 2)) {
            $targetsToBuild = $nukeVersions
        } else {
            $targetsToBuild = @($nukeVersions[$selectedNukeIdx])
        }
    }

    $pluginsToBuild = @()

    # If Plugin filter was provided via parameter or CLI
    if ($PluginFilter -ne "") {
        if ($PluginFilter -ieq "ALL") {
            $pluginsToBuild = $PluginRegistry
        } else {
            $matched = $PluginRegistry | Where-Object { $_.Id -ieq $PluginFilter -or $_.Name -ieq $PluginFilter }
            if ($matched) {
                $pluginsToBuild = @($matched)
            } else {
                Write-Host "[ERROR] No plugin matches filter: '$PluginFilter'" -ForegroundColor Red
                Pause-Prompt "Press [Enter] to return..."
                return
            }
        }
    } else {
        # ----------------------------------------------------------------------
        # Submenu 2: Select Plugin(s) to Build
        # ----------------------------------------------------------------------
        $pluginMenuOptions = @("ALL PLUGINS (Build entire 14-tool suite)")
        foreach ($p in $PluginRegistry) {
            $pluginMenuOptions += "$($p.Name) - $($p.Description)"
        }
        $pluginMenuOptions += "Return to Nuke Selection"

        $selectedPluginIdx = Show-InteractiveMenu `
            -Title "SELECT PLUGIN TO BUILD" `
            -SubTitle "Targeting: $(($targetsToBuild | ForEach-Object { $_.Name }) -join ', ')" `
            -Options $pluginMenuOptions

        if ($selectedPluginIdx -lt 0 -or $selectedPluginIdx -eq ($pluginMenuOptions.Count - 1)) {
            return
        }

        if ($selectedPluginIdx -eq 0) {
            $pluginsToBuild = $PluginRegistry
        } else {
            $pluginsToBuild = @($PluginRegistry[$selectedPluginIdx - 1])
        }
    }

    # --------------------------------------------------------------------------
    # Build Execution Loop
    # --------------------------------------------------------------------------
    Safe-ClearHost
    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "                         STARTING PLUGIN BUILD                          " -ForegroundColor Yellow
    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "Nuke Targets : $(($targetsToBuild | ForEach-Object { $_.Name }) -join ', ')" -ForegroundColor Gray
    Write-Host "Plugins      : $(($pluginsToBuild | ForEach-Object { $_.Name }) -join ', ')" -ForegroundColor Gray
    Write-Host "Deploy Root  : $(Join-Path $CompiledRoot 'WIN')" -ForegroundColor Gray
    Write-Host "========================================================================" -ForegroundColor Cyan

    $totalSuccess = 0
    $totalFailed = 0
    $overallTimer = [System.Diagnostics.Stopwatch]::StartNew()

    foreach ($nukeTarget in $targetsToBuild) {
        Write-Host ""
        Write-Host "========================================================================" -ForegroundColor Yellow
        Write-Host " Target: $($nukeTarget.Name) -> COMPILED/WIN/$($nukeTarget.FolderTag)/" -ForegroundColor Yellow
        Write-Host "========================================================================" -ForegroundColor Yellow

        foreach ($plugin in $pluginsToBuild) {
            $ok = Build-SinglePlugin -Plugin $plugin -NukeTarget $nukeTarget -CMakeExe $cmakeExe
            if ($ok) { $totalSuccess++ } else { $totalFailed++ }
        }
    }

    # Ensure root COMPILED/init.py exists
    $rootInit = Join-Path $CompiledRoot "init.py"
    if (-not (Test-Path $rootInit)) {
        Write-Host "[INFO] Root COMPILED/init.py is preserved and verified." -ForegroundColor Gray
    }

    $overallTimer.Stop()
    $failColor = if ($totalFailed -gt 0) { "Red" } else { "Green" }
    Write-Host ""
    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "                          BUILD RUN FINISHED                            " -ForegroundColor Yellow
    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "  Total Completed : $totalSuccess" -ForegroundColor Green
    Write-Host "  Total Failed    : $totalFailed" -ForegroundColor $failColor
    Write-Host "  Total Duration  : $($overallTimer.Elapsed.TotalSeconds.ToString('F1'))s" -ForegroundColor Gray
    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host ""
    Pause-Prompt "Build complete! Press [Enter] to return to menu..."
}

# ==============================================================================
# MENU 2: Cleanup (Uninstall dependencies & intermediate build files)
# ==============================================================================

function Invoke-Cleanup {
    Safe-ClearHost
    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "  MENU 2: CLEANUP DEPENDENCIES & INTERMEDIATE BUILD ARTIFACTS           " -ForegroundColor Yellow
    Write-Host "========================================================================" -ForegroundColor Cyan
    Write-Host "Workspace Root : $WorkspaceRoot (relative to build.bat)" -ForegroundColor Gray
    Write-Host ""
    Write-Host "The following will be cleanly removed:" -ForegroundColor Yellow
    Write-Host "  [X] Local Virtual Environment (.venv/)" -ForegroundColor Red
    Write-Host "  [X] Temporary Build Directories (_build_tmp/, build*/, cmake-build-*/)" -ForegroundColor Red
    Write-Host "  [X] Intermediate Compiler Outputs (*.obj, *.ilk, *.pdb outside COMPILED/)" -ForegroundColor Red
    Write-Host "  [X] CMake Caches (CMakeCache.txt, CMakeFiles/)" -ForegroundColor Red
    Write-Host "  [X] Python Bytecode Caches (__pycache__/, *.pyc, .pytest_cache/)" -ForegroundColor Red
    Write-Host ""
    Write-Host "STRICTLY PRESERVED:" -ForegroundColor Green
    Write-Host "  [O] COMPILED/ directory and all precompiled binaries will NOT be touched." -ForegroundColor Green
    Write-Host "  [O] Upstream source code and external vendor dependencies will NOT be touched." -ForegroundColor Green
    Write-Host ""

    $confirm = if (-not [Console]::IsInputRedirected) {
        Read-Host "Proceed with cleanup? (Type 'y' to confirm, or [Enter] to cancel)"
    } else {
        'y'
    }

    if ($confirm -ne 'y' -and $confirm -ne 'Y') {
        Write-Host "Cleanup cancelled." -ForegroundColor Yellow
        Start-Sleep -Seconds 1
        return
    }

    Write-Host ""
    Write-Host "[1/5] Removing .venv ... " -NoNewline -ForegroundColor Cyan
    if (Test-Path $VenvDir) {
        try {
            Remove-Item -Recurse -Force $VenvDir -ErrorAction Stop
            Write-Host "REMOVED" -ForegroundColor Green
        } catch {
            Write-Host "FAILED: $_" -ForegroundColor Red
        }
    } else {
        Write-Host "NOT PRESENT" -ForegroundColor DarkGray
    }

    Write-Host "[2/5] Removing temporary build directories (_build_tmp, build*, etc.) ... " -NoNewline -ForegroundColor Cyan
    $buildDirs = Get-ChildItem $WorkspaceRoot -Directory | Where-Object {
        $_.Name -match '^(_build_tmp|build.*|cmake-build.*|out)$' -and $_.Name -ne 'COMPILED'
    }
    foreach ($bd in $buildDirs) {
        Remove-Item -Recurse -Force $bd.FullName -ErrorAction SilentlyContinue
    }
    Write-Host "REMOVED ($($buildDirs.Count) directories)" -ForegroundColor Green

    Write-Host "[3/5] Cleaning CMake caches and generated files ... " -NoNewline -ForegroundColor Cyan
    $cmakeCaches = Get-ChildItem $WorkspaceRoot -Recurse -Filter "CMakeCache.txt" -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch '\\COMPILED\\' }
    $cmakeDirs = Get-ChildItem $WorkspaceRoot -Recurse -Directory -Filter "CMakeFiles" -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch '\\COMPILED\\' }
    foreach ($cc in $cmakeCaches) { Remove-Item -Force $cc.FullName -ErrorAction SilentlyContinue }
    foreach ($cd in $cmakeDirs) { Remove-Item -Recurse -Force $cd.FullName -ErrorAction SilentlyContinue }
    Write-Host "REMOVED ($($cmakeCaches.Count + $cmakeDirs.Count) items)" -ForegroundColor Green

    Write-Host "[4/5] Cleaning Python caches (__pycache__, *.pyc) ... " -NoNewline -ForegroundColor Cyan
    $pycaches = Get-ChildItem $WorkspaceRoot -Recurse -Directory -Filter "__pycache__" -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch '\\COMPILED\\' }
    $pytestcaches = Get-ChildItem $WorkspaceRoot -Recurse -Directory -Filter ".pytest_cache" -ErrorAction SilentlyContinue
    $pycFiles = Get-ChildItem $WorkspaceRoot -Recurse -Filter "*.pyc" -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch '\\COMPILED\\' }
    foreach ($pc in $pycaches) { Remove-Item -Recurse -Force $pc.FullName -ErrorAction SilentlyContinue }
    foreach ($pt in $pytestcaches) { Remove-Item -Recurse -Force $pt.FullName -ErrorAction SilentlyContinue }
    foreach ($pf in $pycFiles) { Remove-Item -Force $pf.FullName -ErrorAction SilentlyContinue }
    Write-Host "REMOVED ($($pycaches.Count + $pytestcaches.Count + $pycFiles.Count) items)" -ForegroundColor Green

    Write-Host "[5/5] Cleaning intermediate object/pdb files ... " -NoNewline -ForegroundColor Cyan
    $objFiles = Get-ChildItem $WorkspaceRoot -Recurse -Include "*.obj","*.ilk" -ErrorAction SilentlyContinue | Where-Object {
        $_.FullName -notmatch '\\COMPILED\\' -and $_.FullName -notmatch '\\dependencies\\'
    }
    foreach ($obj in $objFiles) { Remove-Item -Force $obj.FullName -ErrorAction SilentlyContinue }
    Write-Host "REMOVED ($($objFiles.Count) files)" -ForegroundColor Green

    Write-Host ""
    Write-Host "[SUCCESS] Cleanup complete! COMPILED binaries are strictly preserved." -ForegroundColor Green
    Write-Host ""
    Pause-Prompt "Press [Enter] to return to menu..."
}

# ==============================================================================
# MENU 3: Exit
# ==============================================================================

function Invoke-ExitMenu {
    Safe-ClearHost
    Write-Host "Exiting Nuke Plugin Build Manager. Happy compositing!" -ForegroundColor Cyan
    exit 0
}

# ==============================================================================
# Main Interactive Menu Loop / Direct Parameter Dispatch
# ==============================================================================

# Direct non-interactive execution if menu argument was supplied
if ($MenuChoice -ne "") {
    switch ($MenuChoice) {
        "0" { Invoke-InstallRequirements; exit 0 }
        "1" { Invoke-BuildPlugins -NukeFilter $TargetNuke -PluginFilter $TargetPlugin; exit 0 }
        "2" { Invoke-Cleanup; exit 0 }
        "3" { Invoke-ExitMenu }
        "guide" { Show-ToolchainGuide; exit 0 }
        "-g" { Show-ToolchainGuide; exit 0 }
        default {
            Write-Host "Unknown menu option: $MenuChoice (valid: 0, 1, 2, 3, guide)" -ForegroundColor Red
            exit 1
        }
    }
}

# Interactive loop
$mainMenuOptions = @(
    "Install Requirements & Toolchain Audit (.venv, pip, dependencies)",
    "Build Plugins (Nuke version submenu & selective builds)",
    "Cleanup (Remove .venv & intermediate build files, preserve COMPILED/)",
    "Exit"
)

while ($true) {
    $sel = Show-InteractiveMenu `
        -Title "NUKE PLUGIN BUILD & ENVIRONMENT MANAGER" `
        -SubTitle "Generic GPU & CPU Acceleration | Windows x64" `
        -Options $mainMenuOptions

    switch ($sel) {
        0 { Invoke-InstallRequirements }
        1 { Invoke-BuildPlugins }
        2 { Invoke-Cleanup }
        3 { Invoke-ExitMenu }
        999 { Show-ToolchainGuide }
        -1 { Invoke-ExitMenu }
    }
}

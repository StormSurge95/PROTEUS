param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Debug", "Release")]
    [string]$Config
)

$ErrorActionPreference = "Stop"

function Get-VcpkgTriplet {
    $runnerOs = $env:RUNNER_OS
    $runnerArch = $env:RUNNER_ARCH

    switch ($runnerOs) {
        "Windows" {
            if ($runnerArch -eq "ARM64") { return "arm64-windows" }
            return "x64-windows"
        }
        "Linux" {
            if ($runnerArch -eq "ARM64") { return "arm64-linux" }
            return "x64-linux"
        }
        "macOS" {
            if ($runnerArch -eq "ARM64") { return "arm64-osx" }
            return "x64-osx"
        }
        default {
            throw "Unsupported RUNNER_OS='$runnerOs' RUNNER_ARCH='$runnerArch'"
        }
    }
}

function Invoke-Checked {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,

        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    Write-Host "> $FilePath $($Arguments -join ' ')"
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE"
    }
}

$workspace = $env:GITHUB_WORKSPACE
if ([string]::IsNullOrWhiteSpace($workspace)) {
    throw "GITHUB_WORKSPACE is not set"
}

$vcpkgRoot = if ($env:VCPKG_ROOT) {
    $env:VCPKG_ROOT
} else {
    Join-Path $workspace "vcpkg"
}

$triplet = Get-VcpkgTriplet
$buildDir = Join-Path $workspace "build-ci"
$toolchainFile = Join-Path $vcpkgRoot "scripts/buildsystems/vcpkg.cmake"

if (Test-Path $buildDir) {
    Remove-Item -LiteralPath $buildDir -Recurse -Force
}

Invoke-Checked -FilePath "vcpkg" -Arguments @(
    "install",
    "sdl3:$triplet",
    "openssl:$triplet",
    "nlohmann-json:$triplet"
)

Invoke-Checked -FilePath "cmake" -Arguments @(
    "-S", $workspace,
    "-B", $buildDir,
    "-G", "Ninja Multi-Config",
    "-DPROTEUS_BUILD_TESTS=ON",
    "-DPROTEUS_NES_BUILD_TESTS=OFF",
    "-DPROTEUS_GBA_BUILD_TESTS=OFF",
    "-DCMAKE_TOOLCHAIN_FILE=$toolchainFile",
    "-DVCPKG_TARGET_TRIPLET=$triplet"
)

Invoke-Checked -FilePath "cmake" -Arguments @(
    "--build", $buildDir,
    "--config", $Config
)

Invoke-Checked -FilePath "ctest" -Arguments @(
    "--test-dir", $buildDir,
    "-C", $Config,
    "--output-on-failure",
    "--verbose",
    "-LE", "timing"
)
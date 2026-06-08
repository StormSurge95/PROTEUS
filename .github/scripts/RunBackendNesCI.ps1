param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("Debug", "Release")]
    [string]$Config
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

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

function Download-File {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Url,

        [Parameter(Mandatory = $true)]
        [string]$OutFile
    )

    Write-Host "> download $Url -> $OutFile"
    Invoke-WebRequest -Uri $Url -OutFile $OutFile
}

$workspace = $env:GITHUB_WORKSPACE
if ([string]::IsNullOrWhiteSpace($workspace)) {
    throw "GITHUB_WORKSPACE is not set"
}

$triplet = Get-VcpkgTriplet

$vcpkgRoot = Join-Path $workspace "vcpkg"
$buildDir = Join-Path $workspace "build-ci"
$sstDir = Join-Path $workspace "sst"
$romDir = Join-Path $workspace "roms"
$toolchainFile = Join-Path $vcpkgRoot "scripts/buildsystems/vcpkg.cmake"

$detCpu = Join-Path $romDir "cpu.nes"
$detVid = Join-Path $romDir "litewall2.nes"
$detSnd = Join-Path $romDir "volumes.nes"

if (Test-Path $buildDir) {
    Remove-Item -LiteralPath $buildDir -Recurse -Force
}
if (Test-Path $sstDir) {
    Remove-Item -LiteralPath $sstDir -Recurse -Force
}
if (Test-Path $romDir) {
    Remove-Item -LiteralPath $romDir -Recurse -Force
}

New-Item -ItemType Directory -Path $sstDir | Out-Null
New-Item -ItemType Directory -Path $romDir | Out-Null

Invoke-Checked -FilePath "vcpkg" -Arguments @(
    "install",
    "sdl3:$triplet",
    "openssl:$triplet",
    "nlohmann-json:$triplet"
)

for ($i = 0; $i -le 255; $i++) {
    $hexUpper = "{0:X2}" -f $i
    $hexLower = "{0:x2}" -f $i
    $url = "https://raw.githubusercontent.com/SingleStepTests/65x02/main/nes6502/v1/$hexLower.json"
    $outFile = Join-Path $sstDir "$hexUpper.json"
    Download-File -Url $url -OutFile $outFile
}

Download-File -Url "https://raw.githubusercontent.com/christopherpow/nes-test-roms/master/blargg_nes_cpu_test5/cpu.nes" -OutFile $detCpu
Download-File -Url "https://raw.githubusercontent.com/christopherpow/nes-test-roms/master/volume_tests/volumes.nes" -OutFile $detAudio
Download-File -Url "https://raw.githubusercontent.com/christopherpow/nes-test-roms/master/blargg_litewall/litewall2.nes" -OutFile $detFrame

Invoke-Checked -FilePath "cmake" -Arguments @(
    "-S", $workspace,
    "-B", $buildDir,
    "-G", "Ninja Multi-Config",
    "-DPROTEUS_BUILD_TESTS=OFF",
    "-DPROTEUS_NES_BUILD_TESTS=ON",
    "-DPROTEUS_GBA_BUILD_TESTS=OFF",
    "-DCMAKE_TOOLCHAIN_FILE=$toolchainFile",
    "-DVCPKG_TARGET_TRIPLET=$triplet",
    "-DSSTROOT=$sstDir",
    "-DNES_DET_CPU=$detCpu",
    "-DNES_DET_FRAME=$detVid",
    "-DNES_DET_AUDIO=$detSnd"
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
    "-L", "emulator:nes",
    "-L", "type:sst",
    "-L", "kind:cpu",
    "-j"
)

Invoke-Checked -FilePath "ctest" -Arguments @(
    "--test-dir", $buildDir,
    "-C", $Config,
    "--output-on-failure",
    "--verbose",
    "-L", "emulator:nes",
    "-L", "type:determinism"
)
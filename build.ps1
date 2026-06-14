param(
    [Parameter(Mandatory = $true)]
    [Alias("cfg")]
    [ValidateSet("d", "debug", "r", "release")]
    [string]$Config,
    
    [Alias("c")]
    [switch]$clean
)

$Config = switch ($Config.ToLowerInvariant()) {
    "d" { "Debug" }
    "debug" { "Debug" }
    "r" { "Release" }
    "release" { "Release" }
}

$ErrorActionPreference = "Stop"

$buildDir = Join-Path $pwd "build-ninja"
$detCpu = "C:\ROMS\TESTS\NES\blargg_nes_cpu_test5-cpu.nes"
$detPpu = "C:\ROMS\TESTS\NES\litewall2.nes"
$detApu = "C:\ROMS\TESTS\NES\volumes.nes"

if ($clean -and (Test-Path $buildDir)) {
    Write-Host "> removing $buildDir..."
    Remove-Item -LiteralPath $buildDir -Recurse -Force
}

$cfgArgs = @(
    "-S", ".",
    "-B", $buildDir,
    "-G", "Ninja Multi-Config",
    "-DPROTEUS_BUILD_TESTS=ON",
    "-DPROTEUS_NES_BUILD_TESTS=ON",
    "-DPROTEUS_GBA_BUILD_TESTS=OFF",    # TODO: change this once GBA emulator is testable
    "-DCMAKE_TOOLCHAIN_FILE=C:\devenv\vcpkg\scripts\buildsystems\vcpkg.cmake",
    "-DVCPKG_TARGET_TRIPLET=x64-windows",
    "-DSSTROOT=C:\devenv\PROTEUS\SSTs\NES",
    "-DCMAKE_CXX_COMPILER=clang-cl",
    "-DNES_DET_CPU=$detCpu",
    "-DNES_DET_FRAME=$detPpu",
    "-DNES_DET_AUDIO=$detApu"
)

$bldArgs = @(
    "--build", $buildDir,
    "--config", $Config
)

cmake @cfgArgs
cmake @bldArgs
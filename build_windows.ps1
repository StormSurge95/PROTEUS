param(
    [Alias("D","debug")]
    [switch]$dbg,
    [Alias("R","release")]
    [switch]$rls,
    [Alias("T","test")]
    [switch]$tst
)

$ErrorActionPreference = "Stop"

if ($dbg -and $rls) {
    throw "Use either -debug (-D) or -release (-R), not both."
}

$config = if ($rls) { "release" } else { "debug" }

if ($config -eq "debug") {
    $configPreset = if ($tst) { "windows-x64-debug-tests" } else { "windows-x64-debug" }
    $buildPreset = if ($tst) { "build-windows-debug-tests" } else { "build-windows-debug" }
    $testPreset = "test-windows-debug"
} else {
    $configPreset = if ($tst) { "windows-x64-release-tests" } else { "windows-x64-release" }
    $buildPreset = if ($tst) { "build-windows-release-tests" } else { "build-windows-release" }
    $testPreset = "test-windows-release"
}

cmake --preset $configPreset
cmake --build --preset $buildPreset

if ($tst) {
    ctest --preset $testPreset
}
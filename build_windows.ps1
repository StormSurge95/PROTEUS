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
    $configPreset = if ($tst) { "ninja-debug-tests-x64" } else { "ninja-debug-x64" }
    $buildPreset = if ($tst) { "ninja-build-debug-tests" } else { "ninja-build-debug" }
    $testPreset = "ninja-test-debug"
} else {
    $configPreset = if ($tst) { "ninja-release-tests-x64" } else { "ninja-release-x64" }
    $buildPreset = if ($tst) { "ninja-build-release-tests" } else { "ninja-build-release" }
    $testPreset = "ninja-test-release"
}

cmake --preset $configPreset
cmake --build --preset $buildPreset

if ($tst) {
    ctest --preset $testPreset
}
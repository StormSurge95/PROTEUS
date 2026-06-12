param(
    [Alias("cfg")]
    [ValidateSet(
        "d", "debug",           # run tests on "debug" build
        "r", "release"          # run tests on "release" build
    )]
    [string]$Config = "debug",  # "debug" by default

    [Alias("b")]
    [switch]$BuildFirst,        # optional param to force rebuild before attempting to test

    [Alias("c")]
    [switch]$clean,             # optional param to force clean rebuild

    [Alias("l")]            # label param can be passed via "-L" OR "-Label"
    [ValidateSet(
        "all",      # label to run ALL avaiable ctests
        "frontend", # label to run any frontend tests (currently this limits to "plugin lifecycle" tests)
        "nes",      # label to run all NES console tests (SST and determinism)
        "nes:det",  # label to run only NES determinism tests
        "nes:sst",  # label to run only NES SST tests
        "time"      # label to run specifically timing/performance related tests
    )]
    [string]$Label = "all"
)

# parse config param so that we can process it more easily later
$Config = switch ($Config.ToLowerInvariant()) {
    "d" { "Debug" }
    "debug" { "Debug" }
    "r" { "Release" }
    "release" { "Release" }
}

$ErrorActionPreference = "Stop"

$repoRoot = $PSScriptRoot
$buildDir = Join-Path $repoRoot "build-ninja"
$buildScript = Join-Path $repoRoot ".\build.ps1"

if ($BuildFirst -or -not (Test-Path $buildDir)) {
    $buildArgs = @{
        cfg = $Config.ToLowerInvariant()
    }
    if ($clean) { $buildArgs.Add("clean", $true) }

    & $buildScript @buildArgs
    
    if ($LASTEXITCODE -ne 0) {
        throw "build.ps1 failed with exit code $LASTEXITCODE"
    }
}

# create an array to hold our compiled list of arguments for the `ctest` command
$ctestArgs = [System.Collections.Generic.List[string]]::new()
$ctestArgs.Add("--test-dir")            # provide the compiled test-directory
$ctestArgs.Add($buildDir)
$ctestArgs.Add("-C")
$ctestArgs.Add($Config)
$ctestArgs.Add("--output-on-failure")   # make ctest share the output of the ran test upon failure
$ctestArgs.Add("--verbose")             # make ctest output verbose

if ($Label -eq "time") {
    ctest @ctestArgs -L timing
    
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} else {
    $labels = [System.Collections.Generic.List[string]]::new()

    if ($Label -eq "all") {
        $labels.Add("plugin")
        $labels.Add("nes:sst")
        $labels.Add("nes:det")
    } elseif ($Label -eq "frontend") {
        $labels.Add("plugin")
    } elseif ($Label -eq "nes") {
        $labels.Add("nes:sst")
        $labels.Add("nes:det")
    } else {
        $labels.Add($Label)
    }

    if ($labels.Contains("plugin")) {
        ctest @ctestArgs -L plugin
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }

    if ($labels.Contains("nes:sst")) {
        ctest @ctestArgs -L emulator:nes -L type:sst -j
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }

    if ($labels.Contains("nes:det")) {
        ctest @ctestArgs -L emulator:nes -L type:determinism
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
}

Write-Host "All tests successfully passed" -ForegroundColor Green
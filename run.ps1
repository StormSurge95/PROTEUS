param(
    [Alias("cfg")]
    [ValidateSet("d", "debug", "r", "release")]
    [string]$config = "debug",

    [Alias("b")]
    [switch]$buildFirst,

    [Alias("c")]
    [switch]$clean,

    [Alias("h")]
    [switch]$headless,

    [Alias("station")]
    [string]$core,

    [Alias("rom")]
    [string]$romPath,

    [Alias("i")]
    [string]$inputSpec
)

$Config = switch ($Config) {
    "d"         { "Debug" }
    "debug"     { "Debug" }
    "r"         { "Release" }
    "release"   { "Release" }
}

$ErrorActionPreference = "Stop"

$repoRoot = $PSScriptRoot
$buildDir = Join-Path $repoRoot "build-ninja"
$buildScript = Join-Path $repoRoot "./build.ps1"

if ($buildFirst -or -not (Test-Path $buildDir)) {
    $buildArgs = @{
        cfg = $Config.ToLowerInvariant()
    }
    if ($clean) { $buildArgs.Add("clean", $true) }

    & $buildScript @buildArgs
    
    if ($LASTEXITCODE -ne 0) {
        throw "build.ps1 failed with exit code $LASTEXITCODE"
    }
}

$execPath = Join-Path $PSScriptRoot "build-ninja/bin/$Config/Proteus.exe"

$romPathNotDefined = [string]::IsNullOrWhiteSpace($romPath)
$coreNotDefined = [string]::IsNullOrWhiteSpace($core)

if (-not $Headless) {
    if (-not $romPathNotDefined) {
        Write-Warning "Not running in headless mode; -romPath parameter ignored"
    }

    & $execPath
} else {
    if ($romPathNotDefined) {
        Write-Host "Run Failed: ROM path required if running in headless mode." -ForegroundColor Red
        exit 1
    } elseif ($coreNotDefined) {
        Write-Host "Run Failed: Core required if running in headless mode." -ForegroundColor Red
        exit 1
    }

    $execArgs = @("-headless", "-core", $core, "-rom", $romPath)

    if (-not [string]::IsNullOrWhiteSpace($inputSpec)) {
        $execArgs += @("-input", $inputSpec)
    }

    & $execPath @execArgs
}
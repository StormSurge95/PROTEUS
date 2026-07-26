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

function Get-File {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Url,

        [Parameter(Mandatory = $true)]
        [string]$OutFile
    )

    Write-Host "> download $Url -> $OutFile"

    $ProgressPreference = "SilentlyContinue"
    Invoke-WebRequest -Uri $Url -OutFile $OutFile
}

# call 'vcpkg' and ensure that the required packages are installed
Invoke-Checked -FilePath "vcpkg" -Arguments @(
    "install",
    "sdl3:x64-windows",
    "openssl:x64-windows",
    "nlohmann-json:x64-windows"
)

# path to directory containing all SST files
$sstpath = Join-Path -Path $PSScriptRoot -ChildPath "sst"

# create sst directory if it does not already exist
if (!(Test-Path -Path $sstpath)) {
    Write-Host "'$sstpath' path not found; creating directory..."
    New-Item -Path $sstpath -ItemType Directory -Force | Out-Null
}

# array for our sst url/path objects
$ssts = @()

# for each SST...
for ($i = 0; $i -le 255; $i++) {
    # get the uppercase hex value
    $hexUpper = "{0:X2}" -f $i
    # get the lowercase hex value
    $hexLower = "{0:x2}" -f $i
    # construct the file url using the lowercase hex
    $url = "https://raw.githubusercontent.com/SingleStepTests/65x02/main/nes6502/v1/$hexLower.json"
    # construct the system file path using the uppercase hex
    $outFile = Join-Path -Path $sstpath -ChildPath "$hexUpper.json"

    # if the file does not already exist on the system, add an object for it to our array
    if (!(Test-Path $outFile)) {
        $ssts += [PSCustomObject]@{
            url = $url
            outFile = $outFile
        }
    }
}

# if we have SST objects to download, download them in parallel cuz 256 downloads is a lot
if ($ssts.Count -gt 0) {
    Write-Host "> downloading $($ssts.Count) SST file(s)..."

    # only perform a maximum of 8 at once so we don't bog down the system
    $maxJobs = 8

    # jobs array for keeping everything nice and neat
    $jobs = @()

    # for each SST object...
    foreach ($obj in $ssts) {
        # manual throttling to ensure that we only have a max of 8 jobs at any given time
        while (($jobs | Where-Object { $_.State -eq "Running" }).Count -ge $maxJobs) {
            Start-Sleep -Milliseconds 250
        }

        # get the properties within the SST object
        $url = $obj.url
        $nam = $obj.outFile

        # Start the actual download job
        $job = Start-Job -ScriptBlock {
            param($url, $nam)
            $ProgressPreference = "SilentlyContinue" # this apparently is needed to prevent logging the progress of each individual WebRequest
            Invoke-WebRequest -Uri $url -OutFile $nam
        } -ArgumentList $url, $nam

        # add the new job to our jobs array
        $jobs += $job
    }

    # wait and ensure all jobs are completed
    $jobs | Wait-Job | Out-Null
    $jobs | Receive-Job | Out-Null
    # remove completed jobs from system to free up resources
    $jobs | Remove-Job

    Write-Host "> SST downloads complete"
} else {
    Write-Host "> All SST files present"
}

# path to directory containing all ROM files
$rompath = Join-Path -Path $PSScriptRoot -ChildPath "roms"

# create rom directory if it does not already exist
if (!(Test-Path -Path $rompath)) {
    Write-Host "'$rompath' path not found; creating directory..."
    New-Item -Path $rompath -ItemType Directory -Force | Out-Null
}

# list of consoles needing their own rom directory
$cons = @(
    "NES" # currently limited to just NES
)

# create each subdirectory if it does not already exist
$cons | ForEach-Object {
    $conpath = Join-Path -Path $rompath -ChildPath $_
    if (!(Test-Path -Path "$conpath")) {
        Write-Host "'$conpath' path not found; creating directory..."
        New-Item -Path $conpath -ItemType Directory -Force | Out-Null
    }
}

# array of test rom objects to use for determinism testing; each object is composed of a url string to download FROM and a path string to download TO
$detRoms = @(
    # NES CPU Determinism
    [PSCustomObject]@{
        pat = "$rompath/NES/cpu.nes"
        url = "https://raw.githubusercontent.com/christopherpow/nes-test-roms/master/blargg_nes_cpu_test5/cpu.nes"
    },
    # NES Video Determinism
    [PSCustomObject]@{
        pat = "$rompath/NES/litewall2.nes"
        url = "https://raw.githubusercontent.com/christopherpow/nes-test-roms/master/blargg_litewall/litewall2.nes"
    },
    # NES Sound Determinism
    [PSCustomObject]@{
        pat = "$rompath/NES/volumes.nes"
        url = "https://raw.githubusercontent.com/christopherpow/nes-test-roms/master/volume_tests/volumes.nes"
    }
)

# for each test rom object...
foreach ($obj in $detRoms) {
    # download the test rom if it does not already exist on the system
    if (!(Test-Path -Path $obj.pat)) {
        Get-File -Url $obj.url -OutFile $obj.pat
    } else {
        Write-Host "> $(Split-Path -Leaf $obj.pat) already present"
    }
}
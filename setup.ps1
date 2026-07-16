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
    Invoke-WebRequest -Uri $Url -OutFile $OutFile
}

Invoke-Checked -FilePath "vcpkg" -Arguments @(
    "install",
    "sdl3:x64-windows",
    "openssl:x64-windows",
    "nlohmann-json:x64-windows"
)

for ($i = 0; $i -le 255; $i++) {
    $hexUpper = "{0:X2}" -f $i
    $hexLower = "{0:x2}" -f $i
    $url = "https://raw.githubusercontent.com/SingleStepTests/65x02/main/nes6502/v1/$hexLower.json"
    $outFile = "./sst/$hexUpper.json"

    if (-not (Test-Path $outFile)) {
        Get-File -Url $url -OutFile $outFile
    }
}

$detCpu = "./roms/NES/cpu.nes"
$detVid = "./roms/NES/litewall2.nes"
$detSnd = "./roms/NES/volumes.nes"

Get-File -Url "https://raw.githubusercontent.com/christopherpow/nes-test-roms/master/blargg_nes_cpu_test5/cpu.nes" -OutFile $detCpu
Get-File -Url "https://raw.githubusercontent.com/christopherpow/nes-test-roms/master/volume_tests/volumes.nes" -OutFile $detSnd
Get-File -Url "https://raw.githubusercontent.com/christopherpow/nes-test-roms/master/blargg_litewall/litewall2.nes" -OutFile $detVid
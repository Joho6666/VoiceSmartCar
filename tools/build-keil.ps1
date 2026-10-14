param(
    [Parameter(Mandatory=$true)][ValidateSet('DryRun_12T','DryRun_6T','Hardware')][string]$TargetName,
    [switch]$Rebuild
)
$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$c51 = 'C:\Keil_v5\C51\BIN\C51.exe'
$bl51 = 'C:\Keil_v5\C51\BIN\BL51.EXE'
$oh51 = 'C:\Keil_v5\C51\BIN\OH51.EXE'
foreach ($tool in @($c51,$bl51,$oh51)) { if (!(Test-Path -LiteralPath $tool)) { throw "Keil C51 tool missing: $tool" } }
$build = Join-Path $project (Join-Path 'build' $TargetName)
New-Item -ItemType Directory -Path $build -Force | Out-Null
$log = Join-Path $build 'build.log'
if ($Rebuild) { Get-ChildItem -LiteralPath $build -File | Remove-Item -Force }
$defines = if ($TargetName -eq 'DryRun_6T') { 'VOICECAR_6T=1' } else { 'VOICECAR_12T=1' }
if ($TargetName -eq 'Hardware') {
    $defines += ',VOICECAR_HARDWARE=1'
    $guardLog = Join-Path $build 'hardware-guard.log'
    $guardArgs = @((Join-Path $project 'firmware\bsp_motor.c'), "OBJECT($(Join-Path $build 'hardware-guard.obj'))", "INCDIR($(Join-Path $project 'firmware');C:\Keil_v5\C51\INC)", "DEFINE($defines)")
    $guardOutput = & $c51 @guardArgs 2>&1 | Out-String
    $guardCode = $LASTEXITCODE
    Set-Content -LiteralPath $guardLog -Value $guardOutput -Encoding UTF8
    $guardListing = Join-Path $project 'firmware\bsp_motor.LST'
    if (Test-Path -LiteralPath $guardListing) { Move-Item -LiteralPath $guardListing -Destination (Join-Path $build 'hardware-guard.LST') -Force }
    $configText = Get-Content -LiteralPath (Join-Path $project 'firmware\board_config.h') -Raw
    $requiredGates = @('Hardware motor HAL is not implemented')
    if ($configText -match '(?m)^#define\s+BOARD_IO_MAPPING_CONFIRMED\s+0') { $requiredGates += 'verify MCU-to-L293D pin mapping' }
    if ($configText -match '(?m)^#define\s+BOARD_SAFE_LOGIC_RAIL_CONFIRMED\s+0') { $requiredGates += 'verify the regulated MCU logic rail' }
    if ($configText -match '(?m)^#define\s+BOARD_RESET_MOTOR_OFF_CONFIRMED\s+0') { $requiredGates += 'verify motor outputs stay off during reset' }
    if ($guardCode -eq 0 -or @($requiredGates | Where-Object { $guardOutput -notmatch [regex]::Escape($_) }).Count -gt 0) {
        throw "Hardware configuration guard did not reject the unverified target as expected; see $guardLog"
    }
    Write-Output "EXPECTED BLOCK: Hardware target rejected until board checks are confirmed and the measured motor HAL is implemented."
    Write-Output "Guard evidence: $guardLog"
    return
}
$sources = Get-ChildItem -LiteralPath (Join-Path $project 'firmware') -Filter '*.c' | Sort-Object Name
$objects = @()
$logLines = @("Target: $TargetName", 'Compiler: Keil C51 9.59', "Defines: $defines", '')
foreach ($source in $sources) {
    $object = Join-Path $build ($source.BaseName + '.obj')
    $objects += $object
    $compilerArgs = @($source.FullName, "OBJECT($object)", "INCDIR($(Join-Path $project 'firmware');C:\Keil_v5\C51\INC)", "DEFINE($defines)")
    $output = & $c51 @compilerArgs 2>&1 | Out-String
    $code = $LASTEXITCODE
    $logLines += "--- C51 $($source.Name) (exit $code) ---", $output
    $listing = Join-Path $source.DirectoryName ($source.BaseName + '.LST')
    if (Test-Path -LiteralPath $listing) {
        $listingDestination = Join-Path $build ($source.BaseName + '.LST')
        Move-Item -LiteralPath $listing -Destination $listingDestination -Force
    }
    if ($code -ne 0 -or $output -match '\*\*\* ERROR C\d+') {
        Set-Content -LiteralPath $log -Value ($logLines -join "`r`n") -Encoding UTF8
        throw "C51 compile failed for $($source.Name); see $log"
    }
}
$abs = Join-Path $build 'VoiceSmartCar.abs'
$map = Join-Path $build 'VoiceSmartCar.m51'
$hex = Join-Path $build 'VoiceSmartCar.hex'
$objectList = $objects -join ','
$linkArg = "$objectList TO $abs PRINT($map) RAMSIZE(256)"
$linkOutput = & $bl51 $linkArg 2>&1 | Out-String
$linkCode = $LASTEXITCODE
$logLines += "--- BL51 (exit $linkCode; warnings may yield nonzero exit) ---", $linkOutput
if (!(Test-Path -LiteralPath $abs) -or $linkOutput -notmatch '0 ERROR\(S\)') {
    Set-Content -LiteralPath $log -Value ($logLines -join "`r`n") -Encoding UTF8
    throw "BL51 link failed; see $log"
}
$hexOutput = & $oh51 $abs "HEXFILE($hex)" 2>&1 | Out-String
$hexCode = $LASTEXITCODE
$logLines += "--- OH51 (exit $hexCode) ---", $hexOutput
if ($hexCode -ne 0 -or !(Test-Path -LiteralPath $hex)) {
    Set-Content -LiteralPath $log -Value ($logLines -join "`r`n") -Encoding UTF8
    throw "OH51 HEX conversion failed; see $log"
}
$logLines += '', (Select-String -LiteralPath $map -Pattern 'Program Size:' | ForEach-Object { $_.Line })
Set-Content -LiteralPath $log -Value ($logLines -join "`r`n") -Encoding UTF8
Write-Output "PASS Keil C51 compile/link/HEX: $hex"
Write-Output "MAP: $map"
Write-Output "LOG: $log"

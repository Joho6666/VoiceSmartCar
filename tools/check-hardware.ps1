$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$schema = Join-Path $project 'hardware\VoiceSmartCar.kicad_sch'
$cli = 'E:\kidcad\bin\kicad-cli.exe'
if (!(Test-Path -LiteralPath $schema)) { throw "Missing editable schematic: $schema" }
if (!(Test-Path -LiteralPath $cli)) { throw "KiCad CLI not found: $cli" }
$out = Join-Path $project 'hardware'
Push-Location $out
try {
    & $cli sch export pdf '.\VoiceSmartCar.kicad_sch' -o '.\VoiceSmartCar-block-diagram.pdf'
    if ($LASTEXITCODE -ne 0) { throw "PDF export failed ($LASTEXITCODE)." }
    & $cli sch export netlist '.\VoiceSmartCar.kicad_sch' -o '.\VoiceSmartCar-review.net'
    if ($LASTEXITCODE -ne 0) { throw "Netlist export failed ($LASTEXITCODE)." }
    & $cli sch erc '.\VoiceSmartCar.kicad_sch' -o '.\VoiceSmartCar-erc.rpt'
    $ercCode = $LASTEXITCODE
    if (Test-Path '.\VoiceSmartCar-erc.rpt') { Get-Content '.\VoiceSmartCar-erc.rpt' }
    if ($ercCode -ne 0) { throw "Schematic ERC reports findings ($ercCode); see the report." }
    Write-Output 'PASS KiCad export/parse/ERC for the editable block-level review sheet.'
    Write-Output 'NOTE The sheet deliberately contains no asserted PCB nets; exported connectivity is not hardware verification.'
} finally { Pop-Location }

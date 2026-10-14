param()
$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$build = Join-Path $project 'build'
New-Item -ItemType Directory -Path $build -Force | Out-Null
$cl = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe'
if (!(Test-Path -LiteralPath $cl)) { throw 'The inspected Visual Studio C++ compiler was not found at its recorded path.' }
$vc = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207'
$sdkBase = 'C:\Program Files (x86)\Windows Kits\10'
$sdkVersion = (Get-ChildItem -LiteralPath (Join-Path $sdkBase 'Include') -Directory | Sort-Object Name -Descending | Select-Object -First 1 -ExpandProperty Name)
if (!$sdkVersion) { throw "Windows SDK headers not found under $sdkBase" }
$includeDirs = @((Join-Path $vc 'include'),'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\VS\include',(Join-Path $sdkBase "Include\$sdkVersion\ucrt"),(Join-Path $sdkBase "Include\$sdkVersion\shared"),(Join-Path $sdkBase "Include\$sdkVersion\um"),(Join-Path $sdkBase "Include\$sdkVersion\winrt"),(Join-Path $sdkBase "Include\$sdkVersion\cppwinrt"))
$libDirs = @((Join-Path $vc 'lib\x64'),(Join-Path $sdkBase "Lib\$sdkVersion\ucrt\x64"),(Join-Path $sdkBase "Lib\$sdkVersion\um\x64"))
foreach ($directory in ($includeDirs + $libDirs)) { if (!(Test-Path -LiteralPath $directory)) { throw "Required compiler directory missing: $directory" } }
$sources = @('tests\test_core.c','firmware\car_core.c','firmware\voice_protocol.c','firmware\motion_controller.c','firmware\action_sequence.c')
$output = Join-Path $build 'test_core.exe'
$log = Join-Path $build 'host-test.log'
Push-Location $project
try {
    $args = @('/nologo','/W4','/TC','/DVOICECAR_HOST_TEST=1','/Ifirmware')
    foreach ($directory in $includeDirs) { $args += "/I$directory" }
    $args += $sources + @("/Fe:$output",'/Fo:build\','/link')
    foreach ($directory in $libDirs) { $args += "/LIBPATH:$directory" }
    & $cl @args *> $log
    if ($LASTEXITCODE -ne 0) { Get-Content -LiteralPath $log; throw "Host test compile failed ($LASTEXITCODE)." }
    & $output *>> $log
    if ($LASTEXITCODE -ne 0) { Get-Content -LiteralPath $log; throw "Host test failed ($LASTEXITCODE)." }
    Get-Content -LiteralPath $log
    Write-Output "PASS host tests; log: $log"
} finally { Pop-Location }

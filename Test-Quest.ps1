$ErrorActionPreference='Stop'
$vswhere='C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(!$vs){throw 'MSVC C tools are required for the host math tests.'}
$vcvars=Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
$testDir=Join-Path $PSScriptRoot 'build\tests'
New-Item -ItemType Directory -Force -Path $testDir | Out-Null
# cmd is only used to initialize the compiler environment, never for filesystem deletion/moving.
$batch=Join-Path $testDir 'test.cmd'
@"
@echo off
call "$vcvars" >nul
cd /d "$testDir"
cl /nologo /std:c11 /W4 /I"$PSScriptRoot\quest" /I"$PSScriptRoot\upstream\engine" "$PSScriptRoot\tests\test_vr.c" "$PSScriptRoot\quest\quest_scene.c" /Fe:test_vr.exe
if errorlevel 1 exit /b 1
test_vr.exe
"@ | Set-Content -LiteralPath $batch -Encoding ascii
& $env:ComSpec /c $batch
if($LASTEXITCODE -ne 0){throw 'VR tests failed.'}

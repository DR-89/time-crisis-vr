param([string]$Rom = "$env:USERPROFILE\Downloads\timecris.zip", [int]$Jobs=2, [switch]$BundleRoms)
$ErrorActionPreference='Stop'
$bundledPython=Join-Path $env:USERPROFILE '.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
$python=if(Test-Path -LiteralPath $bundledPython){$bundledPython}else{'python'}
$extra=@()
if($BundleRoms){$extra+='--bundle-roms'}
& $python (Join-Path $PSScriptRoot 'tools\build.py') --rom $Rom --jobs $Jobs @extra
if($LASTEXITCODE -ne 0){throw "Quest build failed ($LASTEXITCODE)"}

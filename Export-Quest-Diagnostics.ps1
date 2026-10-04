param([string]$Serial,[string]$Output)
$ErrorActionPreference='Stop'
$adb=if(Get-Command adb -ErrorAction SilentlyContinue){(Get-Command adb).Source}else{'C:\platform-tools\adb.exe'}
$targetArgs=@();if($Serial){$targetArgs=@('-s',$Serial)}
if(!$Output){$Output=Join-Path $PSScriptRoot ('artifacts\diagnostics-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))}
New-Item -ItemType Directory -Force -Path $Output | Out-Null
& $adb @targetArgs get-state
if($LASTEXITCODE -ne 0){throw 'Connect the Quest and allow USB debugging.'}
foreach($name in @('timecris-vr.log','timecris-vr-previous.log','last-session.inputs','previous-session.inputs','quest-options.cfg')){
    $lines=& $adb @targetArgs shell run-as org.timecrisis.quest cat "files/$name" 2>$null
    if($LASTEXITCODE -eq 0){
        # Input replays begin with SS22REC; a Windows PowerShell UTF-8 BOM breaks that header.
        [IO.File]::WriteAllText((Join-Path $Output $name),($lines -join "`n")+"`n",[Text.UTF8Encoding]::new($false))
    }
}
& $adb @targetArgs shell dumpsys activity exit-info org.timecrisis.quest | Set-Content -Encoding utf8 (Join-Path $Output 'exit-info.txt')
& $adb @targetArgs shell dumpsys package org.timecrisis.quest | Set-Content -Encoding utf8 (Join-Path $Output 'package.txt')
Write-Host "Diagnostics saved to $Output"

param([string]$Serial)
$ErrorActionPreference='Stop'
$apk=Join-Path $PSScriptRoot 'artifacts\TimeCrisisVR-quest3-debug.apk'
if(!(Test-Path -LiteralPath $apk)){throw 'APK missing. Run Build-Quest.ps1 first.'}
$adb=if(Get-Command adb -ErrorAction SilentlyContinue){(Get-Command adb).Source}else{'C:\platform-tools\adb.exe'}
$targetArgs=@()
if($Serial){$targetArgs=@('-s',$Serial)}
& $adb @targetArgs get-state
if($LASTEXITCODE -ne 0){throw 'Connect Quest via USB, enable developer mode and accept the USB debugging prompt in the headset.'}
& $adb @targetArgs install -r $apk
if($LASTEXITCODE -ne 0){throw 'APK installation failed.'}
& $adb @targetArgs shell am start -n org.timecrisis.quest/.LauncherActivity
if($LASTEXITCODE -ne 0){throw 'APK launch failed.'}

param(
    [Parameter(Mandatory=$true)][string]$Rom,
    [string]$Apk,
    [string]$Serial
)
$ErrorActionPreference='Stop'
if(!(Test-Path -LiteralPath $Rom -PathType Leaf)){throw 'ROM ZIP not found.'}
$adb=if(Get-Command adb -ErrorAction SilentlyContinue){(Get-Command adb).Source}elseif(Test-Path -LiteralPath 'C:\platform-tools\adb.exe'){'C:\platform-tools\adb.exe'}else{throw 'Install Android platform-tools and add adb to PATH.'}
$targetArgs=@()
if($Serial){$targetArgs=@('-s',$Serial)}
function Invoke-Adb {
    & $adb @targetArgs @args
    if($LASTEXITCODE -ne 0){throw "ADB failed: $($args[0])"}
}
Invoke-Adb get-state
if($Apk){
    if(!(Test-Path -LiteralPath $Apk -PathType Leaf)){throw 'APK not found.'}
    Invoke-Adb install -r $Apk
}
Invoke-Adb shell am force-stop org.timecrisis.quest
$destination='/sdcard/Android/data/org.timecrisis.quest/files'
Invoke-Adb shell mkdir -p $destination
Invoke-Adb push $Rom "$destination/timecris.zip"
Invoke-Adb shell am start -n org.timecrisis.quest/.LauncherActivity
Write-Host 'The app now verifies and imports your own ROM set. No ROM files are downloaded.'

"""Exercise the report exporter with a fake ADB; never touches a real device."""
from pathlib import Path
import os, subprocess, tempfile, zipfile

ROOT = Path(__file__).resolve().parents[1]
MOCK = r'''
$global:LASTEXITCODE=0
$line=$args -join ' '
Add-Content -LiteralPath $env:TCVR_FAKE_ADB_CALLS -Value $line
if($line -eq 'devices -l'){
    'List of devices attached'
    'PHONE device product:phone model:SM_G780G device:phone'
    if($env:TCVR_FAKE_ADB_MODE -ne 'phone'){'QUEST device product:eureka model:Quest_3 device:eureka'}
    if($env:TCVR_FAKE_ADB_MODE -eq 'multiple'){'OTHER device product:hollywood model:Quest_2 device:hollywood'}
}elseif($line -eq '-s QUEST shell run-as org.timecrisis.quest id'){
    if($env:TCVR_FAKE_ADB_MODE -eq 'uninstalled'){$global:LASTEXITCODE=1}else{'uid=10123(u0_a123)'}
}elseif($line -eq '-s QUEST shell run-as org.timecrisis.quest cat files/timecris-vr.log'){
    if($env:TCVR_FAKE_ADB_MODE -eq 'nolog'){$global:LASTEXITCODE=1}else{'[TRAP] f12345 at 0x001234 -> 0x00056789: fixture'}
}elseif($line -eq '-s QUEST shell run-as org.timecrisis.quest cat files/last-session.inputs'){
    'SS22REC 1 TC FFFF';'12345 10 200 0 0 17D A3 0'
}elseif($line -eq '-s QUEST shell run-as org.timecrisis.quest cat files/quest-options.cfg'){
    'gun_pitch=-45'
}elseif($line -eq '-s QUEST shell dumpsys activity exit-info org.timecrisis.quest'){
    'reason=EXIT_SELF status=5'
}elseif($line -eq '-s QUEST shell dumpsys package org.timecrisis.quest'){
    'versionName=0.8.3'
}else{$global:LASTEXITCODE=1}
'''
with tempfile.TemporaryDirectory(prefix='tcvr-diagnostics-') as temp:
    directory = Path(temp)
    mock = directory / 'mock-adb.ps1'
    mock.write_text(MOCK, encoding='utf-8')
    env = os.environ.copy()
    env['TCVR_FAKE_ADB_CALLS'] = str(directory / 'calls.txt')
    def run(mode, output, *extra):
        env['TCVR_FAKE_ADB_MODE'] = mode
        return subprocess.run(['powershell.exe', '-NoProfile', '-ExecutionPolicy', 'Bypass',
                               '-File', str(ROOT / 'Export-Quest-Diagnostics.ps1'),
                               '-Adb', str(mock), '-Output', str(output), *extra],
                              env=env, capture_output=True, text=True, timeout=30)
    report = directory / 'report with spaces'
    result = run('quest', report)
    assert result.returncode == 0, result.stdout + result.stderr
    with zipfile.ZipFile(str(report)+'.zip') as z:
        files = {Path(name).name: z.read(name) for name in z.namelist() if not name.endswith('/')}
        assert files['last-session.inputs'].startswith(b'SS22REC 1 TC '), 'No UTF-8 BOM in replay'
        assert files['quest-options.cfg'] == b'gun_pitch=-45\n'
        assert b'previous-session.inputs' in files['report.txt']
        assert b'versionName=0.8.3' in files['package.txt']
        assert len(files) == 6, files.keys()
    original = Path(str(report)+'.zip').read_bytes()
    assert run('quest', report).returncode != 0
    assert Path(str(report)+'.zip').read_bytes() == original, 'Existing diagnostics must not be overwritten'
    for mode in ('phone', 'multiple', 'uninstalled', 'nolog'):
        target = directory / mode
        result = run(mode, target)
        assert result.returncode != 0, (mode, result.stdout)
        assert not Path(str(target)+'.zip').exists(), mode
    assert run('multiple', directory / 'explicit', '-Serial', 'QUEST').returncode == 0
    calls = (directory / 'calls.txt').read_text()
    assert '-s PHONE' not in calls and 'install' not in calls and 'am start' not in calls
print('PASS: Quest-only selection, explicit device, missing optional files, BOM-free replay, complete ZIP, no false-success/overwrite/device mutations.')

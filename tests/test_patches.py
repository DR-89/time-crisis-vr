"""All Quest patches reproduce from the pinned upstream and are idempotent."""
from pathlib import Path
import importlib.util, subprocess, tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('patch_upstream',ROOT/'tools/patch_upstream.py')
patch=importlib.util.module_from_spec(spec);spec.loader.exec_module(patch)
names=('geo_hw.h','geo_hw.c','ss22_gl.c','quad_gl.c','tex_bake.c','sprite_hw.c','text_hw.c')
with tempfile.TemporaryDirectory(prefix='tcvr-patch-') as directory:
    patch.ROOT=Path(directory);(patch.ROOT/'engine').mkdir()
    for name in names:
        source=subprocess.check_output(['git','show','HEAD:engine/'+name],cwd=ROOT/'upstream')
        (patch.ROOT/'engine'/name).write_bytes(source)
    patch.main()
    once={name:(patch.ROOT/'engine'/name).read_text() for name in names}
    patch.main()
    for name in names:
        assert once[name]==(patch.ROOT/'engine'/name).read_text(),('not idempotent',name)
        assert once[name]==(ROOT/'upstream/engine'/name).read_text(),('working copy mismatch',name)
print('PASS: 7 patched engine files reproduce from HEAD; second application changes nothing.')

"""Import QuestRetroDepth's baked CC0 pistol, preserving its moving slide parts."""
from pathlib import Path
import argparse, hashlib, json, re, struct


def convert(source, destination):
    data = source.read_bytes()
    vertices = []
    for row in re.findall(r'\{([^{}]+)\}', data.decode('utf-8')):
        values = re.findall(r'([-+0-9.eE]+)f', row)
        if len(values) == 10:
            vertices.append(list(map(float, values)))
    if len(vertices) != 3216 or {v[9] for v in vertices} != {0, 1, 2}:
        raise ValueError('Expected the 1,072-triangle QuestRetroDepth pistol')
    # Center the actual barrel opening on the controller aim ray. Preserve scale
    # and -Z forward; the original header places the barrel above its origin.
    front = min(v[2] for v in vertices)
    opening = [v for v in vertices if abs(v[2] - front) < 1e-6]
    center = [(min(v[i] for v in opening) + max(v[i] for v in opening)) / 2 for i in (0, 1)]
    for v in vertices:
        v[0] -= center[0]
        v[1] -= center[1]
    packed = b'TCGUN002' + struct.pack('<2I3f', len(vertices), len(vertices), 0, 0, front)
    packed += b''.join(struct.pack('<10f', *v) for v in vertices)
    packed += struct.pack('<' + 'I' * len(vertices), *range(len(vertices)))
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(packed)
    report = dict(model='Lowpoly Pistol', author='FireWarden', license='CC0',
                  source='QuestRetroDepth app/src/main/cpp/lightgun_model.h',
                  original_url='https://opengameart.org/content/lowpoly-pistol',
                  source_sha256=hashlib.sha256(data).hexdigest(),
                  asset_sha256=hashlib.sha256(packed).hexdigest(), format='TCGUN002',
                  vertices=len(vertices), triangles=len(vertices)//3,
                  parts={'0': 'body', '1': 'trigger', '2': 'slide and sights'},
                  source_barrel_center_xy=center, muzzle_m=[0, 0, front],
                  slide_travel_m=0.034, recoil_duration_ms=150,
                  bounds_m=[[min(v[i] for v in vertices) for i in range(3)],
                            [max(v[i] for v in vertices) for i in range(3)]])
    destination.with_suffix('.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(f'Imported {len(vertices)//3} triangles with separate slide/sights')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('destination', type=Path)
    args = parser.parse_args()
    convert(args.source, args.destination)

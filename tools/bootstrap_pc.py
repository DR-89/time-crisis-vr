"""Pinned Windows/OpenXR build dependencies, kept local to this checkout."""
from concurrent.futures import ThreadPoolExecutor
from bootstrap import fetch

PACKAGES = [
    ('llvm-mingw', 'https://github.com/mstorsjo/llvm-mingw/releases/download/20260922/llvm-mingw-20260922-ucrt-x86_64.zip', None),
    ('openxr-pc', 'https://github.com/KhronosGroup/OpenXR-SDK/releases/download/release-1.1.43/OpenXR.Loader.1.1.43.nupkg', None),
    ('zlib', 'https://github.com/madler/zlib/archive/refs/tags/v1.3.1.zip', None),
]

if __name__ == '__main__':
    with ThreadPoolExecutor(max_workers=3) as pool:
        list(pool.map(fetch, PACKAGES))

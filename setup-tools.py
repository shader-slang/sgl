import os
import sys
import shutil
import subprocess
from pathlib import Path
from urllib.parse import urlparse
from urllib.request import urlretrieve

PROJECT_DIR = Path(__file__).parent

DOWNLOAD_DIR = PROJECT_DIR / "tools/download"
INSTALL_DIR = PROJECT_DIR / "tools/host"

PLATFORM = "windows-x64"

ARCHIVE_EXTENSIONS = [".zip", ".tar.gz", ".tar.bz2", ".7z"]


def is_archive_path(path: Path):
    return path.suffix in ARCHIVE_EXTENSIONS


def download_progress_hook(count, block_size, total_size):
    if total_size <= 0:
        return
    percent = int(count * block_size * 100 / total_size)
    sys.stdout.write(f"\r{percent}%")
    sys.stdout.flush()


def download_file(url, path: Path):
    try:
        urlretrieve(url, path, reporthook=download_progress_hook)
    except Exception as e:
        raise Exception(f"Failed to download {url} ({e}))")


def decompress_file(_7za_path: Path, path: Path, dest_dir: Path, strip: int):
    args = [str(_7za_path), "x", "-y", "-spe", "-o" + str(dest_dir), str(path)]
    print(f"Decompressing {path} ...")
    subprocess.run(args)

    # strip directories
    if strip > 0:
        dirs = list(dest_dir.iterdir())
        if len(dirs) == 1:
            print(f"Stripping {dirs[0].stem} ...")
            for path in dirs[0].iterdir():
                shutil.move(str(path), str(dest_dir))
            shutil.rmtree(str(dirs[0]))


class Context:
    def __init__(self, platform: str, download_dir: Path, install_dir: Path):
        self.platform = platform
        self.download_dir = download_dir
        self.install_dir = install_dir
        self._7za_path = install_dir / "7za" / "7za"

        if not self.download_dir.exists():
            self.download_dir.mkdir(parents=True)
        if not self.install_dir.exists():
            self.install_dir.mkdir(parents=True)


class Package:
    def __init__(self):
        self.name = None
        self.version = None
        self.infos = {}

    def setup(self, ctx: Context):
        self.info = self.infos[ctx.platform]
        self.url = self.info["url"]
        self.basename = os.path.basename(urlparse(self.url).path)
        self.download_path = ctx.download_dir / self.basename
        self.sha512_path = ctx.download_dir / f"{self.basename}.sha512"
        self.install_dir = ctx.install_dir / self.name

    def download(self, ctx: Context):
        print(f"Downloading {self.url} ...")
        download_file(self.url, self.download_path)
        print("")

    def deploy(self, ctx: Context):
        self.create_install_dir(ctx)
        if is_archive_path(self.download_path):
            self.decompress_to_install_dir(ctx)
        else:
            self.copy_to_install_dir(ctx)

    def create_install_dir(self, ctx: Context):
        if not self.install_dir.exists():
            self.install_dir.mkdir(parents=True)

    def copy_to_install_dir(self, ctx: Context):
        shutil.copy(self.download_path, self.install_dir)
        if "rename" in self.info:
            shutil.move(
                self.install_dir / self.basename,
                self.install_dir / self.info["rename"],
            )

    def decompress_to_install_dir(self, ctx: Context):
        strip = self.info.get("strip", 0)
        decompress_file(ctx._7za_path, self.download_path, self.install_dir, strip)

    def install(self, ctx: Context):
        print(f"Installing package '{self.name}' ...")
        self.setup(ctx)
        self.download(ctx)
        self.deploy(ctx)


class _7za(Package):
    def __init__(self):
        self.name = "7za"
        self.version = None
        self.infos = {
            "windows-x64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/win/x64/7za.exe",
            },
            "linux-x64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/linux/x64/7za",
            },
            "linux-arm64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/linux/arm64/7za",
            },
            "macos-x64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/mac/x64/7za",
            },
            "macos-arm64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/mac/arm64/7za",
            },
        }

    def deploy(self, ctx: Context):
        self.create_install_dir(ctx)
        self.copy_to_install_dir(ctx)


class cmake(Package):
    def __init__(self):
        self.name = "cmake"
        self.version = "3.27.7"
        self.infos = {
            "windows-x64": {
                "url": f"https://github.com/Kitware/CMake/releases/download/v{self.version}/cmake-{self.version}-windows-x86_64.zip",
                "strip": 1,
            },
            "linux-x64": {
                "url": f"https://github.com/Kitware/CMake/releases/download/v{self.version}/cmake-{self.version}-linux-x86_64.tar.gz",
                "strip": 1,
            },
            "linux-arm64": {
                "url": f"https://github.com/Kitware/CMake/releases/download/v{self.version}/cmake-{self.version}-linux-aarch64.tar.gz",
                "strip": 1,
            },
        }


class ninja(Package):
    def __init__(self):
        self.name = "ninja"
        self.version = "1.11.1"
        self.infos = {
            "windows-x64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-win.zip",
            },
            "linux-x64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-linux.zip",
            },
        }


class clang_format(Package):
    def __init__(self):
        self.name = "clang-format"
        self.version = "16"
        self.infos = {
            "windows-x64": {
                "url": f"https://github.com/muttleyxd/clang-tools-static-binaries/releases/download/master-f4f85437/clang-format-{self.version}_windows-amd64.exe",
                "rename": "clang-format.exe",
            },
            "linux-x64": {
                "url": f"https://github.com/muttleyxd/clang-tools-static-binaries/releases/download/master-f4f85437/clang-format-{self.version}_linux-amd64",
                "rename": "clang-format",
            },
        }


ctx = Context(PLATFORM, DOWNLOAD_DIR, INSTALL_DIR)

_7za().install(ctx)
cmake().install(ctx)
ninja().install(ctx)
clang_format().install(ctx)

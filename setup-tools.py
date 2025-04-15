import os
import sys
import shutil
import subprocess
from pathlib import Path
from urllib.parse import urlparse
from urllib.request import urlretrieve
from hashlib import sha512

PROJECT_DIR = Path(__file__).parent

DOWNLOAD_DIR = PROJECT_DIR / "tools/download"
INSTALL_DIR = PROJECT_DIR / "tools/host"


def get_platform():
    """
    Returns the platform name.
    """
    if sys.platform.startswith("win"):
        return "windows-x64"
    elif sys.platform.startswith("linux"):
        if os.uname().machine == "aarch64":
            return "linux-arm64"
        else:
            return "linux-x64"
    elif sys.platform.startswith("darwin"):
        if os.uname().machine == "arm64":
            return "macos-arm64"
        else:
            return "macos-x64"
    else:
        raise Exception(f"Unsupported platform: {sys.platform}")


def is_archive_path(path: Path):
    """
    Returns True if the path is an archive file.
    """
    ARCHIVE_EXTENSIONS = [".zip", ".tar.gz", ".tar.bz2", ".7z", ".whl"]
    return any(path.name.endswith(ext) for ext in ARCHIVE_EXTENSIONS)


def is_tar_path(path: Path):
    """
    Returns True if the path is a tar file.
    """
    TAR_EXTENSIONS = [".tar.gz", ".tar.bz2"]
    return any(path.name.endswith(ext) for ext in TAR_EXTENSIONS)


def download_file(url: str, path: Path):
    """
    Download a file from the given URL to the given path.
    """

    last_percent = -1

    def progress_hook(count, block_size, total_size):
        nonlocal last_percent
        if total_size <= 0:
            return
        percent = int(count * block_size * 100 / total_size)
        if percent != last_percent:
            sys.stdout.write(f"\r{percent}%")
            sys.stdout.flush()
            last_percent = percent

    try:
        urlretrieve(url, path, reporthook=progress_hook)
        print("\rDone")
    except Exception as e:
        raise Exception(f"Failed to download {url} ({e}))")


def compute_sha512(path: Path):
    """
    Compute the SHA512 hash of the given file.
    """
    with open(path, "rb") as f:
        return sha512(f.read()).hexdigest()


def write_sha512_file(path: Path, sha512: str):
    """
    Write the SHA512 hash of the given file to a file.
    """
    with open(path, "w") as f:
        f.write(sha512)


def read_sha512_file(path: Path):
    """
    Read the SHA512 hash of the given file from a file.
    """
    with open(path, "r") as f:
        return f.read()


def decompress_7za(_7za_path: Path, path: Path, dest_dir: Path, strip: int):
    """
    Decompress the given archive file to the given directory.
    Optionally strip the root directory from the archive.
    """

    args = [str(_7za_path), "x", "-y", "-spe", "-o" + str(dest_dir), str(path)]
    print(f"Decompressing {path} ...")
    p = subprocess.run(args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if p.returncode != 0:
        raise Exception(f"Failed to decompress {path} ({p.returncode})")

    # strip directories
    if strip > 0:
        dirs = list(dest_dir.iterdir())
        if len(dirs) == 1:
            print(f"Stripping {dirs[0].name} ...")
            for path in dirs[0].iterdir():
                shutil.move(str(path), str(dest_dir))
            shutil.rmtree(str(dirs[0]))


def decompress_tar(path: Path, dest_dir: Path, strip: int):
    """
    Decompress the given tar file to the given directory.
    Optionally strip the root directory from the archive.
    """

    args = ["tar", "xf", str(path), "-C", str(dest_dir)]
    if strip > 0:
        args += ["--strip-components", str(strip)]

    print(f"Decompressing {path} ...")
    p = subprocess.run(args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if p.returncode != 0:
        raise Exception(f"Failed to decompress {path} ({p.returncode})")


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
        self.name: str = None
        self.version: str = None
        self.infos = {}

    def setup(self, ctx: Context):
        self.info = self.infos[ctx.platform]
        self.url = self.info["url"]
        self.basename = os.path.basename(urlparse(self.url).path)
        self.download_path = ctx.download_dir / self.basename
        self.download_sha512_path = ctx.download_dir / f"{self.basename}.sha512"
        self.install_dir = ctx.install_dir / self.name
        self.install_sha512_path = self.install_dir / f"{self.name}.sha512"

    def download(self, ctx: Context):
        print(f"Downloading {self.url} ...")
        if self.download_path.exists() and self.download_sha512_path.exists():
            if compute_sha512(self.download_path) == read_sha512_file(
                self.download_sha512_path
            ):
                print(f"Using cached {self.basename}")
                return
        download_file(self.url, self.download_path)
        sha512 = compute_sha512(self.download_path)
        if "sha512" in self.info:
            if sha512 != self.info["sha512"]:
                raise Exception(
                    f"SHA512 mismatch for {self.basename}:\nExpected: {self.info['sha512']}\nComputed: {sha512}"
                )
        write_sha512_file(self.download_sha512_path, sha512)

    def deploy(self, ctx: Context):
        self.create_install_dir(ctx)
        if is_archive_path(self.download_path):
            self.decompress_to_install_dir(ctx)
        else:
            self.copy_to_install_dir(ctx)
        shutil.copy(self.download_sha512_path, self.install_sha512_path)

    def create_install_dir(self, ctx: Context):
        if self.install_dir.exists():
            shutil.rmtree(self.install_dir)
        if not self.install_dir.exists():
            self.install_dir.mkdir(parents=True)

    def copy_to_install_dir(self, ctx: Context):
        shutil.copy(self.download_path, self.install_dir)
        if "chmod" in self.info:
            subprocess.run(
                ["chmod", self.info["chmod"], self.install_dir / self.basename]
            )
        if "rename" in self.info:
            shutil.move(
                self.install_dir / self.basename,
                self.install_dir / self.info["rename"],
            )

    def decompress_to_install_dir(self, ctx: Context):
        strip = int(self.info.get("strip", 0))
        if is_tar_path(self.download_path):
            decompress_tar(self.download_path, self.install_dir, strip)
        else:
            decompress_7za(ctx._7za_path, self.download_path, self.install_dir, strip)
        use_sub_dir: Path = self.info.get("use_sub_dir", None)
        if use_sub_dir:
            sub_dir = self.install_dir / use_sub_dir
            tmp_dir = self.install_dir.with_name(self.install_dir.name + "_tmp")
            if sub_dir.exists():
                shutil.move(sub_dir, tmp_dir)
                shutil.rmtree(self.install_dir)
                shutil.move(tmp_dir, self.install_dir)

    def install(self, ctx: Context):
        print(f"Installing package '{self.name}' ...")
        self.setup(ctx)
        if (
            self.download_sha512_path.exists()
            and self.install_dir.exists()
            and self.install_sha512_path.exists()
        ):
            download_sha512 = read_sha512_file(self.download_sha512_path)
            install_sha512 = read_sha512_file(self.install_sha512_path)
            if download_sha512 == install_sha512:
                print(f"Skipping, already installed.")
                return
        self.download(ctx)
        self.deploy(ctx)


class _7za(Package):
    def __init__(self):
        self.name = "7za"
        self.version = None
        self.infos = {
            "windows-x64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/win/x64/7za.exe",
                "sha512": "544949f1213817599fdb09dbb9834aeeb370b3f6225c3d835a29797b006bd36aa37b8a246a22204277f40d3865a01bc8d029a531d17d6bb43d9ddd3db7370580",
            },
            "linux-x64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/linux/x64/7za",
                "sha512": "796188635271cbd7dbd6a7f37cb4d4d5b394c8a302dc62008c40b4be507382925eeb8a550ca11e81c791d5dbda238f95dedecbdd0daddf84907c4fa3a9b1ca59",
                "chmod": "+x",
            },
            "linux-arm64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/linux/arm64/7za",
                "chmod": "+x",
            },
            "macos-x64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/mac/x64/7za",
                "chmod": "+x",
            },
            "macos-arm64": {
                "url": "https://github.com/develar/7zip-bin/raw/master/mac/arm64/7za",
                "chmod": "+x",
            },
        }


class cmake(Package):
    def __init__(self):
        self.name = "cmake"
        self.version = "3.27.7"
        self.infos = {
            "windows-x64": {
                "url": f"https://github.com/Kitware/CMake/releases/download/v{self.version}/cmake-{self.version}-windows-x86_64.zip",
                "sha512": "eea422bdf5ec01f3ef9e9c3becb12df59a26cda1089a2ff03c1faa2d7f8d478bd14a8a41bf014c5b8a4653b7bf2b26d3a55a45356550b30ac0cda351ad689497",
                "strip": True,
            },
            "windows-arm64": {
                "url": f"https://github.com/Kitware/CMake/releases/download/v{self.version}/cmake-{self.version}-windows-arm64.zip",
                "sha512": "91a9568b146e32a3880178b6f203414aa8edbc259735c104fe27d79c1eb9061a61b8d6b40cdf173891429b4b42b6d761740bb0bdf9dbe0163bf74e92f0dcee3c",
                "strip": True,
            },
            "linux-x64": {
                "url": f"https://github.com/Kitware/CMake/releases/download/v{self.version}/cmake-{self.version}-linux-x86_64.tar.gz",
                "sha512": "37d63a47d5d5b35a0da201d6486d558f06a23b95d06950729f8f353b1d583b764ddaeb327b5e97a296001ce772bdd3212f249d7ff5332a024cc1c16f35822da3",
                "strip": True,
            },
            "linux-arm64": {
                "url": f"https://github.com/Kitware/CMake/releases/download/v{self.version}/cmake-{self.version}-linux-aarch64.tar.gz",
                "strip": True,
            },
            "macos-x64": {
                "url": f"https://github.com/Kitware/CMake/releases/download/v{self.version}/cmake-{self.version}-macos-universal.tar.gz",
                "strip": True,
            },
            "macos-arm64": {
                "url": f"https://github.com/Kitware/CMake/releases/download/v{self.version}/cmake-{self.version}-macos-universal.tar.gz",
                "strip": True,
            },
        }


class ninja(Package):
    def __init__(self):
        self.name = "ninja"
        self.version = "1.12.1"
        self.infos = {
            "windows-x64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-win.zip",
                "sha512": "d6715c6458d798bcb809f410c0364dabd937b5b7a3ddb4cd5aba42f9fca45139b2a8a3e7fd9fbd88fd75d298ed99123220b33c7bdc8966a9d5f2a1c9c230955f",
            },
            "windows-arm64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-winarm64.zip",
                "sha512": "b1826c422a677f47f9f7e001672ce831791b092e4f1cd84ddf2ea067781c31aa8246f26e91dd66300c23ffa77a8ea29910c48ccf7e4235ff20bccc2d2b6e247b",
            },
            "linux-x64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-linux.zip",
                "sha512": "9c2ad534e7e72e67c608de7784cfbae601095bfca96713731a3f1eca268d66a6302f40c138a4ad97f7e8c902cd3fb05994a175e46fe922295dcc2d1334bf9014",
            },
            "linux-arm64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-linux-aarch64.zip",
                "sha512": "22c46abb7e6d916e11713705f78d093e9b30029cb49cadc65755908ad9f44b3f2548105174cc615a5ef86c4672b366173f18bd04c2d71710a303d952c06db334",
            },
            "macos-x64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-mac.zip",
                "sha512": "4c11f477359c9d1dcda72529a503a59948ec20b368992132e545d6d4f6e3aabfd1d6b1d0f32cf932626037959b24a7bb375ef901e2d002eabadc83a265cbc351",
            },
            "macos-arm64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-mac.zip",
                "sha512": "4c11f477359c9d1dcda72529a503a59948ec20b368992132e545d6d4f6e3aabfd1d6b1d0f32cf932626037959b24a7bb375ef901e2d002eabadc83a265cbc351",
            },
        }


class clang_format(Package):
    def __init__(self):
        self.name = "clang-format"
        self.version = "16.0.6"
        self.infos = {
            "windows-x64": {
                "url": f"https://files.pythonhosted.org/packages/9b/b0/66c1799ef634511829a4c2c419831f40116196b5f4041055e2bc4ea9645b/clang_format-{self.version}-py2.py3-none-win_amd64.whl",
                "sha512": "512a95d26a416a555f261e2e1e707f15f66ae5a5d7815559080c0c7e2bb35fedb6d9fa4b02bff1bd71fd1ff29da29207adeb34baabb0ac494418b46f56edf97d",
                "use_sub_dir": "clang_format/data/bin",
            },
            "linux-x64": {
                "url": f"https://files.pythonhosted.org/packages/83/47/10591237672762b61099011f04f154d5b46c21b4f88979c92331a04edacb/clang_format-{self.version}-py2.py3-none-manylinux_2_17_x86_64.manylinux2014_x86_64.whl",
                "sha512": "9dbb215810dbff4f75614928532f2ee88cf527b11b0834adbaa8d4f3a1940ce9aeb6e44475edbf3097f49bdad8a4b9f4d857c74798ff993b0f49a7dee0b033a3",
                "use_sub_dir": "clang_format/data/bin",
            },
            "linux-arm64": {
                "url": f"https://files.pythonhosted.org/packages/83/47/10591237672762b61099011f04f154d5b46c21b4f88979c92331a04edacb/clang_format-{self.version}-py2.py3-none-manylinux_2_17_x86_64.manylinux2014_x86_64.whl",
                "sha512": "9dbb215810dbff4f75614928532f2ee88cf527b11b0834adbaa8d4f3a1940ce9aeb6e44475edbf3097f49bdad8a4b9f4d857c74798ff993b0f49a7dee0b033a3",
                "use_sub_dir": "clang_format/data/bin",
            },
            "macos-x64": {
                "url": f"https://files.pythonhosted.org/packages/01/fb/8267d7035ec217df109cfde2164a26121413c3e7cd92896b862ce86b947c/clang_format-{self.version}-py2.py3-none-macosx_10_9_universal2.whl",
                "sha512": "705244ca8ba4c3fa9926311deedceebd9afdc6927c8ec38ac3d7083c814788060735b3baa6a8c5d8449737b4f7aedd590471ec18988e4508fc225d0a9a2d4bc9",
                "use_sub_dir": "clang_format/data/bin",
            },
            "macos-arm64": {
                "url": f"https://files.pythonhosted.org/packages/01/fb/8267d7035ec217df109cfde2164a26121413c3e7cd92896b862ce86b947c/clang_format-{self.version}-py2.py3-none-macosx_10_9_universal2.whl",
                "sha512": "705244ca8ba4c3fa9926311deedceebd9afdc6927c8ec38ac3d7083c814788060735b3baa6a8c5d8449737b4f7aedd590471ec18988e4508fc225d0a9a2d4bc9",
                "use_sub_dir": "clang_format/data/bin",
            },
        }


ctx = Context(get_platform(), DOWNLOAD_DIR, INSTALL_DIR)

_7za().install(ctx)
cmake().install(ctx)
ninja().install(ctx)
clang_format().install(ctx)

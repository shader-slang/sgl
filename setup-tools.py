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
    ARCHIVE_EXTENSIONS = [".zip", ".tar.gz", ".tar.bz2", ".7z"]
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

    def progress_hook(count, block_size, total_size):
        if total_size <= 0:
            return
        percent = int(count * block_size * 100 / total_size)
        sys.stdout.write(f"\r{percent}%")
        sys.stdout.flush()

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


def decompress_7za(_7za_path: Path, path: Path, dest_dir: Path, strip: bool):
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


def decompress_tar(path: Path, dest_dir: Path, strip: bool):
    """
    Decompress the given tar file to the given directory.
    Optionally strip the root directory from the archive.
    """

    args = ["tar", "xf", str(path), "-C", str(dest_dir)]
    if strip:
        args += ["--strip-components", "1"]

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
        self.name = None
        self.version = None
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
        strip = self.info.get("strip", 0)
        if is_tar_path(self.download_path):
            decompress_tar(self.download_path, self.install_dir, strip)
        else:
            decompress_7za(ctx._7za_path, self.download_path, self.install_dir, strip)

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
                print(f"Skipping, already intalled.")
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
        self.version = "1.11.1"
        self.infos = {
            "windows-x64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-win.zip",
                "sha512": "a700e794c32eb67b9f87040db7f1ba3a8e891636696fc54d416b01661c2421ff46fa517c97fd904adacdf8e621df3e68ea380105b909ae8b6651a78ae7eb3199",
            },
            "linux-x64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-linux.zip",
                "sha512": "6403dac9196baffcff614fa73ea530752997c8db6bbfbaa0446b4b09d7327e2aa6e8615d1283c961d3bf0df497e85ba86604149f1505ee75f89d600245a45dde",
            },
            "macos-x64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-mac.zip",
                "sha512": "dad33b0918c60bbf5107951a936175b1610b4894a408f4ba4b47a2f5b328fc982a52a2aed6a0cb75028ee4765af5083bea66611c37516826eb0c851366bb4427",
            },
            "macos-arm64": {
                "url": f"https://github.com/ninja-build/ninja/releases/download/v{self.version}/ninja-mac.zip",
                "sha512": "dad33b0918c60bbf5107951a936175b1610b4894a408f4ba4b47a2f5b328fc982a52a2aed6a0cb75028ee4765af5083bea66611c37516826eb0c851366bb4427",
            },
        }


class clang_format(Package):
    def __init__(self):
        self.name = "clang-format"
        self.version = "16"
        self.infos = {
            "windows-x64": {
                "url": f"https://github.com/muttleyxd/clang-tools-static-binaries/releases/download/master-f4f85437/clang-format-{self.version}_windows-amd64.exe",
                "sha512": "5c9768de9adfdf9c181c42509d7fa3c0fb41f99298665067cafb88ebaa1c3b9992b37f90309def32fe9d256e6a4cde43b3a11ec3616a2b34044480a08e44ba7a",
                "rename": "clang-format.exe",
            },
            "linux-x64": {
                "url": f"https://github.com/muttleyxd/clang-tools-static-binaries/releases/download/master-f4f85437/clang-format-{self.version}_linux-amd64",
                "sha512": "b83942b5eda44dcf094e6ae13425ad12a2fa97b106c35eb25863ab11c7bf50854b9660870f645151b65c873011c7feef62f2405dc13d27d0c869b3f3b5dc2cef",
                "rename": "clang-format",
                "chmod": "+x",
            },
            "macos-x64": {
                "url": f"https://github.com/muttleyxd/clang-tools-static-binaries/releases/download/master-f4f85437/clang-format-{self.version}_macosx-amd64",
                "sha512": "2ba0bf4287d33205352174c4dd431960b802fc0a8f43c90263b47411fc02ea013c0afbd350f5b91b17fa7defc3d567910eb4e80b71d0dda47a1d4de0005bac80",
                "rename": "clang-format",
                "chmod": "+x",
            },
            "macos-arm64": {
                "url": f"https://github.com/muttleyxd/clang-tools-static-binaries/releases/download/master-f4f85437/clang-format-{self.version}_macosx-amd64",
                "sha512": "2ba0bf4287d33205352174c4dd431960b802fc0a8f43c90263b47411fc02ea013c0afbd350f5b91b17fa7defc3d567910eb4e80b71d0dda47a1d4de0005bac80",
                "rename": "clang-format",
                "chmod": "+x",
            },
        }


ctx = Context(get_platform(), DOWNLOAD_DIR, INSTALL_DIR)

_7za().install(ctx)
cmake().install(ctx)
ninja().install(ctx)
clang_format().install(ctx)

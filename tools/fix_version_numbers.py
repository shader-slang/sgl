from pathlib import Path
from typing import Callable, Optional
import re


class Version:
    def __init__(self, major: int, minor: int, patch: int):
        super().__init__()
        self.major = major
        self.minor = minor
        self.patch = patch


FileCallback = Callable[[str, Version], str]


class File:
    def __init__(self, path: Path, handler: FileCallback):
        super().__init__()
        self.path = path
        self.handler = handler
        self.initial_content = path.read_text()
        self.modified_content: Optional[str] = None

    def apply_version(self, version: Version):
        modified = self.handler(self.initial_content, version)
        if modified != self.initial_content:
            self.modified_content = modified
            return True
        else:
            return False

    def save(self):
        if self.modified_content:
            self.path.write_text(self.modified_content)


def fix_sgl_h(file: str, version: Version) -> str:
    file = re.sub(
        r"#define SGL_VERSION_MAJOR \d+",
        f"#define SGL_VERSION_MAJOR {version.major}",
        file,
    )
    file = re.sub(
        r"#define SGL_VERSION_MINOR \d+",
        f"#define SGL_VERSION_MINOR {version.minor}",
        file,
    )
    file = re.sub(
        r"#define SGL_VERSION_PATCH \d+",
        f"#define SGL_VERSION_PATCH {version.patch}",
        file,
    )
    return file


def fix_docs_index(file: str, version: Version) -> str:
    file = re.sub(
        r"version = \{\d+\.\d+\.\d+\}",
        f"version = {{{version.major}.{version.minor}.{version.patch}}}",
        file,
    )
    return file


def fix_vcpkg(file: str, version: Version) -> str:
    file = re.sub(
        r'\"version\-string\"\: \"\d+\.\d+\.\d+"',
        f'"version-string": "{version.major}.{version.minor}.{version.patch}"',
        file,
    )
    return file


def run(save: bool = False):
    version = Version(0, 1, 0)
    root = Path(__file__).parent.parent

    changlog = root / "docs/changelog.rst"
    changlog_content = changlog.read_text()

    # find last version in changlog
    match = re.search(r"(\d+)\.(\d+)\.(\d+)", changlog_content)
    if match:
        version = Version(int(match.group(1)), int(match.group(2)), int(match.group(3)))
    else:
        exit(1)

    files = [
        File(root / "src/sgl/sgl.h", fix_sgl_h),
        File(root / "docs/index.rst", fix_docs_index),
        File(root / "README.md", fix_docs_index),
        File(root / "vcpkg.json", fix_vcpkg),
    ]

    for file in files:
        file.apply_version(version)

    if save:
        for file in files:
            file.save()


run(True)

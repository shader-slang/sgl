# Configuration file for the Sphinx documentation builder.

import sys
import shutil
from pathlib import Path

CURRENT_DIR = Path(__file__).parent
shutil.copytree(CURRENT_DIR / "../tutorials", CURRENT_DIR / "src/tutorials", dirs_exist_ok=True)

project = "kali"
copyright = "2024, Simon Kallweit, NVIDIA"
author = "Simon Kallweit"

extensions = ["sphinx_copybutton", "nbsphinx"]

source_suffix = ".rst"
master_doc = "index"
language = "en"

templates_path = ["_templates"]
exclude_patterns = ["CMakeLists.txt", "generated/*"]

# html configuration
html_theme = "furo"
html_title = "kali"
html_static_path = ["_static"]
html_css_files = ["theme_overrides.css"]

# nbsphinx configuration
nbsphinx_execute = "never"


def generate_api(app):
    sys.path.append(str(Path(__file__).parent))
    from generate_api import generate_api

    generate_api()


def setup(app):
    app.connect("builder-inited", generate_api)

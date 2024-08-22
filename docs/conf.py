# Configuration file for the Sphinx documentation builder.

import sys
import shutil
from pathlib import Path
from sphinx.application import Sphinx

project = "sgl"
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
html_title = "sgl"
html_static_path = ["_static"]
html_css_files = ["theme_overrides.css"]
html_theme_options = {
    "light_css_variables": {
        "color-api-background": "#f7f7f7",
    },
    "dark_css_variables": {
        "color-api-background": "#1e1e1e",
    },
}

# nbsphinx configuration
nbsphinx_execute = "never"


def initialize(app: Sphinx):
    # Copy tutorials to src directory.
    print("Copying tutorials to src directory...")
    CURRENT_DIR = Path(__file__).parent
    shutil.copytree(
        CURRENT_DIR / "../tutorials", CURRENT_DIR / "src/tutorials", dirs_exist_ok=True
    )

    # Generate API documentation if sgl module is available.
    try:
        print("Generating API documentation...")
        sys.path.append(str(Path(__file__).parent))
        from generate_api import generate_api

        generate_api()
    except ImportError:
        print("sgl module not available, skipping API documentation generation.")


def setup(app: Sphinx):
    app.connect("builder-inited", initialize)

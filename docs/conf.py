# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import sys
from pathlib import Path

project = "kali"
copyright = "2024, Simon Kallweit, NVIDIA"
author = "Simon Kallweit"

extensions = ["sphinx_copybutton"]

source_suffix = ".rst"
master_doc = "index"
language = "en"

templates_path = ["_templates"]
exclude_patterns = ["CMakeLists.txt", "generated/*"]

html_theme = "furo"
html_title = "kali"
html_static_path = ["_static"]
html_css_files = ["theme_overrides.css"]


def generate_api(app):
    sys.path.append(str(Path(__file__).parent))
    from generate_api import generate_api

    generate_api()


def setup(app):
    app.connect("builder-inited", generate_api)

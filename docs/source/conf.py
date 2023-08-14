# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

project = "kali"
copyright = "2023, Simon Kallweit"
author = "Simon Kallweit"

extensions = [ "breathe", "sphinx_rtd_theme" ]

templates_path = ["_templates"]
exclude_patterns = ["doxygen/*", "CMakeFiles"]

html_theme = "sphinx_rtd_theme"
html_static_path = ["_static"]

# Breathe Configuration
breathe_default_project = "kali"

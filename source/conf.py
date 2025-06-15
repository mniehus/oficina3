# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'oficina3'
copyright = '2025, mniehus'
author = 'mniehus'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'nbsphinx',  # or 'myst_nb'
    'myst_parser', # for markdown
    'sphinx.ext.autodoc', #for python api docs
    'sphinx.ext.mathjax',  # Recommended for HTML output
    # or 'sphinx.ext.imgmath' for image-based math
]

myst_enable_extensions = ["dollarmath"]

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

#html_theme = 'alabaster'
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

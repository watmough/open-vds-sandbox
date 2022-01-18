# Configuration file for building the PDF version of the VDS specification.
# It requires the rhinotype pip package (pip install rhinotype)
#
# The PDF can be built by executing the following in this directory:
# $ python -m sphinx -b rinoh . <outdir>

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
docroot = os.path.abspath('.')


import rinoh.frontend.sphinx

# -- Project information -----------------------------------------------------

project = 'VDS Specification'
copyright = '2022, Bluware Inc.'
author = 'Bluware Inc.'

# The full version, including alpha/beta/rc tags
release = '1.0'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
]

numfig = True # If true, figures, tables and code-blocks are automatically numbered if they have a caption

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The master toctree document.
master_doc = 'index'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# -- Options for Rinoh (PDF) HTML output -------------------------------------------------
rinoh_documents = [
  dict(
    doc    = 'index',
    target = 'VDSSpecification',
    title  = 'VDS Specification',
    logo = 'logo_frontpage.png',
    domain_indices = False,
    template = 'template.rtt'
  ),
]

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'alabaster'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
#html_static_path = ['_static']
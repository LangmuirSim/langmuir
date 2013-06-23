from distutils.core import setup

description = \
"""
A set of python functions and classes for use with the Langmuir charge
transport simulation software.
"""

setup(name         = "LangmuirPython",
      version      = 1.0,
      description  = description,
      author       = "Adam Gagorik",
      author_email = "adam.gagorik@gmail.com",
      packages     = ["langmuir"]
)

# -*- coding: utf-8 -*-
from distutils.core import setup

setup(name         = "Langmuir Python",
      version      = '2.0',
      description  = "Langmuir Python",
      author       = "Adam Gagorik",
      author_email = "adam.gagorik@gmail.com",
      packages     = ["langmuir"],
      requires     = ['numpy', 'scipy', 'matplotlib', 'pandas']
)

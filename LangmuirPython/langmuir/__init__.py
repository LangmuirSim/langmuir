# -*- coding: utf-8 -*-
import common
import regex
import find

try:
    import numpy as np
except ImportError:
    print 'missing: numpy'
    np = None

try:
    import scipy as sp
except ImportError:
    print 'missing: scipy'
    sp = None

try:
    import matplotlib.pyplot as plt
    import matplotlib as mpl
except ImportError:
    print 'missing: matplotlib'
    plt = None
    mpl = None

try:
    import pandas as pd
except ImportError:
    print 'missing: pandas'
    pd = None

try:
    import pint
    units = pint.UnitRegistry()
    Quantity = units.Quantity
except ImportError:
    print 'missing: units'
    Quantity = None
    units = None
    pint = None

try:
    import vtk
except ImportError:
    print 'missing: vtk'
    vtk = None

if not np is None:
    import checkpoint
    import parameters
    import surface
    import grid
else:
    print 'disable: langmuir.checkpoint'
    print 'disable: langmuir.parameters'
    print 'disable: langmuir.grid'

if not sp is None:
    if not plt is None:
        import fit

if not pd is None:
    import datfile
    import analyze

if not pint is None:
    import ivcurve

if not plt is None:
    import plot

if not vtk is None:
    import vtkutils
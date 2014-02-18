import common
import regex
import find

try:
    import numpy as np
except ImportError:
    print 'missing: numpy'

try:
    import scipy as sp
except ImportError:
    print 'missing: scipy'

try:
    import matplotlib.pyplot as plt
    import matplotlib as mpl
except ImportError:
    print 'missing: matplotlib'

try:
    import pandas as pd
except ImportError:
    print 'missing: pandas'

if 'np' in locals():
    import checkpoint
    import parameters
    import surface
    import grid
else:
    print 'disable: langmuir.checkpoint'
    print 'disable: langmuir.parameters'
    print 'disable: langmuir.grid'

if 'sp' in locals():
    if 'plt' in locals():
        import fit

if 'pd' in locals():
    import datfile
    import analyze

if 'plt' in locals():
    import plot
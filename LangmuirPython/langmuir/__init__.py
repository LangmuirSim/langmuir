import checkpoint
import common
import database
import datfile
import find
import parameters
import surface
import plot
import regex
import grid
import sys

try:
    import scipy as sp
except ImportError:
    print >> sys.stderr, 'langmuir: can not load scipy'

try:
    import numpy as np
except ImportError:
    print >> sys.stderr, 'langmuir: can not load numpy'

try:
    import matplotlib as mpl
except ImportError:
    print >> sys.stderr, 'langmuir: can not load mpl'

try:
    import pandas as pd
except ImportError:
    print >> sys.stderr, 'langmuir: can not load pandas'

try:
    import quantities as units
except ImportError:
    print >> sys.stderr, 'langmuir: can not load quantities'

if locals().has_key('sp'):
    import fit
else:
    print >> sys.stderr, 'langmuir: disabled module langmuir.fit'

if locals().has_key('pd'):
    import analyze
else:
    print >> sys.stderr, 'langmuir: disabled module langmuir.analyze'

if locals().has_key('pd') and locals().has_key('units') and locals().has_key('fit'):
    import ivline
else:
    print >> sys.stderr, 'langmuir: disabled module langmuir.ivline'
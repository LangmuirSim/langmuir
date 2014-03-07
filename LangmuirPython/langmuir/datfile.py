"""
.. note::
    Functions for opening Langmuir output files.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import pandas as pd
import collections

Column = collections.namedtuple('Column',
    ['key', 'pytype', 'default', 'units', 'fmt'])

columns = [
    Column('simulation:time'   ,   int,   0,     'ps',     '%d'),
    Column('eSourceL:attempt'  ,   int,   0, 'counts',     '%d'),
    Column('eSourceL:success'  ,   int,   0, 'counts',     '%d'),
    Column('eSourceL:prob'     , float, 0.0,      '%',  '%.15e'),
    Column('eSourceL:rate'     , float, 0.0,   '1/ps',  '%.15e'),
    Column('eSourceL:current'  , float, 0.0,     'nA',  '%.15e'),
    Column('eSourceR:attempt'  ,   int,   0, 'counts',     '%d'),
    Column('eSourceR:success'  ,   int,   0, 'counts',     '%d'),
    Column('eSourceR:prob'     , float, 0.0,      '%',  '%.15e'),
    Column('eSourceR:rate'     , float, 0.0,   '1/ps',  '%.15e'),
    Column('eSourceR:current'  , float, 0.0,     'nA',  '%.15e'),
    Column('eSource:rate'      , float, 0.0,   '1/ps',  '%.15e'),#
    Column('eSource:current'   , float, 0.0,     'nA',  '%.15e'),#
    Column('hSourceL:attempt'  ,   int,   0, 'counts',     '%d'),
    Column('hSourceL:success'  ,   int,   0, 'counts',     '%d'),
    Column('hSourceL:prob'     , float, 0.0,      '%',  '%.15e'),
    Column('hSourceL:rate'     , float, 0.0,   '1/ps',  '%.15e'),
    Column('hSourceL:current'  , float, 0.0,     'nA',  '%.15e'),
    Column('hSourceR:attempt'  ,   int,   0, 'counts',     '%d'),
    Column('hSourceR:success'  ,   int,   0, 'counts',     '%d'),
    Column('hSourceR:prob'     , float, 0.0,      '%',  '%.15e'),
    Column('hSourceR:rate'     , float, 0.0,   '1/ps',  '%.15e'),
    Column('hSourceR:current'  , float, 0.0,     'nA',  '%.15e'),
    Column('hSource:rate'      , float, 0.0,   '1/ps',  '%.15e'),#
    Column('hSource:current'   , float, 0.0,     'nA',  '%.15e'),#
    Column('xSource:attempt'   ,   int,   0, 'counts',     '%d'),
    Column('xSource:success'   ,   int,   0, 'counts',     '%d'),
    Column('xSource:prob'      , float, 0.0,      '%',  '%.15e'),
    Column('xSource:rate'      , float, 0.0,   '1/ps',  '%.15e'),
    Column('xSource:current'   , float, 0.0,     'nA',  '%.15e'),
    Column('source:rate'       , float, 0.0,   '1/ps',  '%.15e'),#
    Column('source:current'    , float, 0.0,     'nA',  '%.15e'),#
    Column('eDrainL:attempt'   , float,   0, 'counts',     '%d'),
    Column('eDrainL:success'   ,   int,   0, 'counts',     '%d'),
    Column('eDrainL:prob'      , float, 0.0,      '%',  '%.15e'),
    Column('eDrainL:rate'      , float, 0.0,   '1/ps',  '%.15e'),
    Column('eDrainL:current'   , float, 0.0,     'nA',  '%.15e'),
    Column('eDrainR:attempt'   ,   int,   0, 'counts',     '%d'),
    Column('eDrainR:success'   ,   int,   0, 'counts',     '%d'),
    Column('eDrainR:prob'      , float, 0.0,      '%',  '%.15e'),
    Column('eDrainR:rate'      , float, 0.0,   '1/ps',  '%.15e'),
    Column('eDrainR:current'   , float, 0.0,     'nA',  '%.15e'),
    Column('eDrain:rate'       , float, 0.0,   '1/ps',  '%.15e'),#
    Column('eDrain:current'    , float, 0.0,     'nA',  '%.15e'),#
    Column('hDrainL:attempt'   ,   int,   0, 'counts',     '%d'),
    Column('hDrainL:success'   ,   int,   0, 'counts',     '%d'),
    Column('hDrainL:prob'      , float, 0.0,      '%',  '%.15e'),
    Column('hDrainL:rate'      , float, 0.0,   '1/ps',  '%.15e'),
    Column('hDrainL:current'   , float, 0.0,     'nA',  '%.15e'),
    Column('hDrainR:attempt'   ,   int,   0, 'counts',     '%d'),
    Column('hDrainR:success'   ,   int,   0, 'counts',     '%d'),
    Column('hDrainR:prob'      , float, 0.0,      '%',  '%.15e'),
    Column('hDrainR:rate'      , float, 0.0,   '1/ps',  '%.15e'),
    Column('hDrainR:current'   , float, 0.0,     'nA',  '%.15e'),
    Column('hDrain:rate'       , float, 0.0,   '1/ps',  '%.15e'),#
    Column('hDrain:current'    , float, 0.0,     'nA',  '%.15e'),#
    Column('drain:rate'        , float, 0.0,   '1/ps',  '%.15e'),#
    Column('drain:current'     , float, 0.0,     'nA',  '%.15e'),#
    Column('xDrain:attempt'    ,   int,   0, 'counts',     '%d'),
    Column('xDrain:success'    ,   int,   0, 'counts',     '%d'),
    Column('xDrain:prob'       , float, 0.0,      '%',  '%.15e'),
    Column('xDrain:rate'       , float, 0.0,   '1/ps',  '%.15e'),
    Column('xDrain:current'    , float, 0.0,     'nA',  '%.15e'),
    Column('electron:count'    ,   int,   0, 'counts',     '%d'),
    Column('hole:count'        ,   int,   0, 'counts',     '%d'),
    Column('carrier:count'     ,   int,   0, 'counts',     '%d'),#
    Column('carrier:difference',   int,   0, 'counts',     '%d'),#
    Column('real:time'         ,   int,   0,     'ms',     '%d'),
    Column('speed'             , float, 0.0,  'ms/ps',  '%.15e') #
]
columns = collections.OrderedDict(((p.key, p) for p in columns))

def fix(dat):
    """
    Fix columns in dataframe.
    """
    dat = dat.fillna(value=0)
    for key, col in columns.iteritems():
        if key in dat.columns:
            dat[key] = dat[key].astype(col.pytype)
        else:
            dat[key] = col.default
            dat[key] = dat[key].astype(col.pytype)
    return dat

def load(handle, **kwargs):
    """
    Load datfile into a Pandas dataframe.
    """
    compression = None
    if isinstance(handle, str):
        if handle.endswith('.gz'):
            compression = 'gzip'
    _kwargs = dict(compression=compression, sep='\s*')
    _kwargs.update(**kwargs)
    dat = pd.read_table(handle, **_kwargs)
    return fix(dat)
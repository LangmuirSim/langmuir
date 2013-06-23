# -*- coding: utf-8 -*-
import re as _re

_rgx_comment = _re.compile(r'\s*#.*$')
_rgx_number  = _re.compile(r'((?:[-\+]?\d+\.?\d*)(?:[eE](?:[-\+])?\d+)?)')
_rgx_sim     = _re.compile(r'sim[_\.]?(\d+)')
_rgx_run     = _re.compile(r'run[_\.]?(\d+)')
_rgx_part    = _re.compile(r'part[_\.]?(\d+)')
_rgx_false   = _re.compile(r'false', _re.IGNORECASE)
_rgx_true    = _re.compile(r'true', _re.IGNORECASE)
_rgx_voltage = _re.compile(r'voltage(?:\.right)?(?:\.left)?[_\.]((?:[-\+]?\d+\.?\d*)(?:[eE](?:[-\+])?\d+)?)')
_rgx_gridx   = _re.compile(r'grid\.x[_\.]((?:[-\+]?\d+\.?\d*)(?:[eE](?:[-\+])?\d+)?)')
_rgx_gridy   = _re.compile(r'grid\.y[_\.]((?:[-\+]?\d+\.?\d*)(?:[eE](?:[-\+])?\d+)?)')
_rgx_gridz   = _re.compile(r'grid\.z[_\.]((?:[-\+]?\d+\.?\d*)(?:[eE](?:[-\+])?\d+)?)')
_rgx_fix     = _re.compile(r'[\.:]+')

def fix_name(string):
    """
    Turn non alpha numeric characters into underscores.

    returns: the fixed string
    """
    return _rgx_fix.sub('_', string)

def number(string, index=1, pytype=float):
    """
    get numbers in a string at index and convert them to type_
    """
    return pytype(_rgx_number.match(string).group(index))

def numbers(string, type_=float):
    """
    find all numbers in a string and convert them to type_
    """
    return [type_(f) for f in _rgx_number.findall(string)]

def sim(string):
    """
    extract the sim as int from a string
    """
    match = _rgx_sim.search(string)
    if match:
        return int(match.group(1))
    return None

def voltage(string):
    """
    extract the votlage as float from a string
    """
    match = _rgx_voltage.search(string)
    if match:
        return float(match.group(1))
    return None

def run(string):
    """
    extract the run as int from a string
    """
    match = _rgx_run.search(string)
    if match:
        return int(match.group(1))
    return None

def part(string):
    """
    extract the part as int from a string
    """
    match = _rgx_part.search(string)
    if match:
        return int(match.group(1))
    return None

def strip_comments(string):
    """
    return string will all comments stripped
    """
    return _rgx_comment.sub('', string)

def fix_boolean(string):
    """
    return string with true/false in correct python format
    """
    fixed = _rgx_false.sub('False', string)
    fixed = _rgx_true.sub('True', string)
    return fixed

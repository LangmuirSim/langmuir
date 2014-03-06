# -*- coding: utf-8 -*-
"""
.. note::
    Functions for string parsing.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import re

regex_comments = re.compile(r'\s*#.*$')
regex_false    = re.compile(r'false', re.IGNORECASE)
regex_true     = re.compile(r'true', re.IGNORECASE)
regex_number   = re.compile(r'((?:[-\+]?\d+\.?\d*)(?:[eE](?:[-\+])?\d+)?)')

def number(string, index=0, pytype=float):
    """
    Get numbers in a string at index and convert them.

    :param string: string
    :param index: which regex group to match
    :param pytype: python type to convert to

    :type string: str
    :type index: int
    :type pytype: type

    >>> print lm.regex.number('asdfasdfa1.231e10')
    12310000000.0
    """
    return lm.regex.numbers(string, pytype)[index]

def numbers(string, pytype=float):
    """
    Find all numbers in a string and convert them.

    :param string: string
    :param index: which regex group to match
    :param pytype: python type to convert to

    :type string: str
    :type index: int
    :type pytype: type

    >>> print lm.regex.numbers('asdfasdfa1.231e10asdf1209asd asd0912 sdaf9 81')
    [12310000000.0, 1209.0, 912.0, 9.0, 81.0]
    """
    return [pytype(f) for f in regex_number.findall(string)]

def strip_comments(string):
    """
    Return string will all comments stripped.

    :param string: string
    :type string: str

    >>> print lm.regex.strip_comments('hello # goodbye')
    hello
    """
    return regex_comments.sub('', string)

def fix_boolean(string):
    """
    Return string with true/false in correct python format.

    :param string: string
    :type string: str

    >>> print lm.regex.fix_boolean('true false True False true false')
    True false True False True false
    """
    fixed = regex_false.sub('False', string)
    fixed = regex_true.sub('True', string)
    return fixed
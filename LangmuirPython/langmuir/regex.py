# -*- coding: utf-8 -*-
"""
@author: adam
"""
import re

regex_comments = re.compile(r'\s*#.*$')
regex_false    = re.compile(r'false', re.IGNORECASE)
regex_true     = re.compile(r'true', re.IGNORECASE)
regex_number   = re.compile(r'((?:[-\+]?\d+\.?\d*)(?:[eE](?:[-\+])?\d+)?)')

def number(string, index=1, pytype=float):
    """
    Get numbers in a string at index and convert them.

    :param string: string
    :param index: which regex group to match
    :param pytype: python type to convert to

    :type string: str
    :type index: int
    :type pytype: type
    """
    return pytype(regex_number.match(string).group(index))

def numbers(string, pytype=float):
    """
    Find all numbers in a string and convert them.

    :param string: string
    :param index: which regex group to match
    :param pytype: python type to convert to

    :type string: str
    :type index: int
    :type pytype: type
    """
    return [pytype(f) for f in regex_number.findall(string)]

def strip_comments(string):
    """
    Return string will all comments stripped.

    :param string: string
    :type string: str
    """
    return regex_comments.sub('', string)

def fix_boolean(string):
    """
    Return string with true/false in correct python format.

    :param string: string
    :type string: str
    """
    fixed = regex_false.sub('False', string)
    fixed = regex_true.sub('True', string)
    return fixed
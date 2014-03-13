"""
table.py
========

.. argparse::
    :module: table
    :func: create_parser
    :prog: table.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import pandas as pd
import collections
import argparse
import os

desc = """
Gather output from pkl files into table.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()
    pkls = lm.find.pkls(work, stub='solar*', r=True)

    table = collections.OrderedDict()
    table['system'] = []
    table['run'] = []

    for pkl in pkls:
        table['system'].append(lm.find.extract_system(pkl))
        table['run'].append(lm.regex.run(pkl))
        data = lm.common.load_pkl(pkl)
        for key, value in data.iteritems():
            if not hasattr(value, '__len__'):
                if not key in table:
                    table[key] = []
                table[key].append(value)

    table = pd.DataFrame(table)
    table.to_csv('table.csv')
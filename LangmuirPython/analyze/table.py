# -*- coding: utf-8 -*-
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

def extract_system(pkl, data):
    return os.path.relpath(pkl, work)
    return os.path.relpath(lm.find.slice_system(pkl), work)

extract = {}
extract['system'] = extract_system
extract['run'   ] = lambda pkl, data: lm.regex.run(pkl)

columns = [
    'system',
    'run',
    'v_oc',
    'i_sc',
    'p_th',
    'j_sc',
    'r_th',
    'i_mp',
    'v_mp',
    'p_mp',
    'j_mp',
    'r_mp',
    'fill',
]

def sort_table(table):
    table = table.set_index('system')

    def sort_key(path):
        items = []
        for item in lm.regex.regex_number.split(path):
            try:
                items.append(float(item))
            except ValueError:
                items.extend(item.split(os.sep))
        return filter(None, items)

    table = table.reindex_axis(sorted(table.index, key=sort_key), axis=0)

    table['system'] = table.index
    table = table.reindex_axis(columns, axis=1)

    table = table.reset_index(drop=True)

    return table

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()
    pkls = lm.find.pkls(work, stub='solar*', r=True)

    table = collections.OrderedDict()
    for column in columns:
        table[column] = []

    for pkl in pkls:
        print os.path.relpath(pkl, work)
        data = lm.common.load_pkl(pkl)
        for c in columns:
            if c in data:
                table[c].append(data[c])
            else:
                table[c].append(extract[c](pkl, data))

    table = pd.DataFrame(table)
    table = sort_table(table)

    table.to_csv('table.csv', index=False)

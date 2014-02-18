# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import argparse
import os

desc = """
Calculate the current in a dat (or pkl) file.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument(dest='ifile', default=None, type=str, nargs='?',
                        metavar='input', help='input file')
    parser.add_argument('--stub', default='calculated', type=str,
                        metavar='stub', help='output file stub')
    parser.add_argument('--csv', action='store_true', default=False,
                        help='save csv file')
    parser.add_argument('--pkl', action='store_true', default=False,
                        help='save pkl file')
    parser.add_argument('--excel', action='store_true', default=False,
                        help='save excel file (slow)')
    opts = parser.parse_args(args)

    if not True in [opts.pkl, opts.csv, opts.excel]:
        raise RuntimeError('must use --pkl or --csv or --xlsx')

    if opts.ifile is None:
        opts.ifile = lm.find.dat(work, at_least_one=True)

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    try:
        assert '.pkl' in opts.ifile
        data = lm.common.load_pkl(opts.ifile)
    except AssertionError:
        data = lm.datfile.load(opts.ifile)

    data = lm.analyze.calculate(data)

    if opts.pkl:
        lm.common.save_pkl(data, '%s.pkl.gz' % opts.stub)

    if opts.csv:
        data.to_csv('%s.csv' % opts.stub, float_format='%.15e', index=False)

    if opts.excel:
        data.to_excel('%s.xlsx' % opts.stub, float_format='%.15f', index=False)
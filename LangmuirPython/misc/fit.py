# -*- coding: utf-8 -*-
"""
Created on Tue Jan 15 10:25:28 2013

@author: adam
"""

from scipy.optimize import fmin_bfgs
import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np
import argparse
import StringIO
import os

#matplotlib parameter
mpl.rcParams['axes.formatter.limits'] = 3, 3

def get_arguments(args=None):
    """
    Get command line arguments

    args : a list of strings, that will override the command line

    returns:
        opts   : the options parsed, (example: opts.input)
        parser : the parser object
    """
    #set up command line parser
    parser, keys = argparse.ArgumentParser(), []
    parser.description = 'fit data in text file to a polynomial, find ' + \
                         'the minimum, and plot the results'
    parser.formatter_class = argparse.RawTextHelpFormatter                         

    parser.add_argument(dest='input' , help='input file name')
    keys.append('input')

    parser.add_argument(dest='output', help='output file name stub',
                        default=None, nargs='?')
    keys.append('output')

    parser.add_argument('--degree', default=6, type=int, metavar='int',
                        help='order of polynomial')
    keys.append('degree')

    parser.add_argument('--x0', type=float, default=0.00, metavar='float',
                        help='initial guess')
    keys.append('x0')

    parser.add_argument('--exclude', type=int, action='append', default=[],
                        help='list of indices of rows to exclude',
                        metavar='0 1 2 ...')
    keys.append('exclude')

    parser.add_argument('--xcol', type=int, default=0, metavar='int',
                        help='index of x-column in data file')
    keys.append('xcol')

    parser.add_argument('--ycol', type=int, default=1, metavar='int',
                        help='index of y-column in data file')
    keys.append('ycol')

    parser.add_argument('--xlabel', type=str, default=None,
                        metavar='str', help='xlabel of plot')
    keys.append('xlabel')

    parser.add_argument('--ylabel', type=str, default=None,
                        metavar='str', help='ylabel of plot')
    keys.append('ylabel')

    parser.add_argument('--hide', default=False, action='store_true',
                        help='do not show plot')
    keys.append('hide')

    #parse command line arguments
    opts = parser.parse_args(args)

    #find the input file stub, to use for naming output files
    if opts.output is None:
        stub, ext = os.path.splitext(opts.input)
        setattr(opts, 'output', stub)
    else:
        stub, ext = os.path.splittext(opts.output)
        setattr(opts, 'output', stub)

    #print the parameters beging used
    epilog = StringIO.StringIO()
    print >> epilog, ''
    for key in keys:
        value = getattr(opts, key)
        print >> epilog, '%-8s = %s' % (key, value)
    print >> epilog, ''
    parser.epilog = epilog.getvalue()

    #print the help message
    parser.print_help()

    return opts, parser

def load_data(file_name, xcol=0, ycol=1, exclude_rows=None):
    """
    Load data from a text file

    xcol         : column index of x-data
    ycol         : column index of y-data
    exclude_rows : list of row indices to exclude

    returns:
        xdata : the x-values
        ydata : the y-values
    """
    if exclude_rows is None:
        exclude_rows = []

    data = []
    with open(file_name) as handle:
        for row_index, line in enumerate(handle):
            if row_index in exclude_rows:
                pass
            else:
                try:
                    tokens = line.strip().split()
                    values = [float(token) for token in tokens]
                    data.append(values)
                except:
                    raise IOError('error reading data, row=%d' % row_index)

    data = np.array(data)
    xdata = data[:,xcol]
    ydata = data[:,ycol]

    return xdata, ydata

def fit_data(xdata, ydata, degree):
    """
    Fit data to a n-degree polynomial

    xdata : the x-values
    ydata : the y-values

    returns:
        fit_coefficients : a list of optimized coefficients
        fit_function     : the resulting fit, represented as a function of x,
    """
    fit_coefficients = np.polyfit(xdata, ydata, degree)
    fit_function     = np.poly1d(fit_coefficients)
    return fit_coefficients, fit_function

def find_minimum(fit_function, x0=2.1):
    """
    Find the minimum of a 2D function

    fit_function : the function to find the minimum of
    x0           : an initial guess

    returns:
        xmin : the minimum x-value
        ymin : the minimum y-value
    """
    xmin = fmin_bfgs(fit_function, x0, disp=False)[0]
    ymin = fit_function(xmin)
    return xmin, ymin

def print_result(handle, fit, xmin, ymin):
    """
    Print the results
    """
    if type(handle) == str:
        handle = open(handle, 'a')

    titles  = ['%15s' % 'xmin', '%15s' % 'ymin']
    titles += ['%15s' % ('c%d' % v) for v in range(len(fit))]

    values  = ['% .8e' % xmin, '% .8e' % ymin]
    values += ['% .8e' % v for v in fit]

    print >> handle, ' '.join(titles)
    print >> handle, ' '.join(values)

def plot_result(xdata, ydata, fit_function, xmin, ymin, xlabel='r ($\AA$)',
                ylabel='E (H)'):
    """
    Plot results using matplotlib

    xdata        : the x-values
    ydata        : the y-values
    fit_function : the function
    xmin         : the minimum x-value
    ymin         : the minimum y-value
    """
    #create figure and axis
    fig, ax = plt.subplots(1, 1)

    #plot data points
    plt.plot(xdata, ydata, 'bo', markersize=8, markerfacecolor='white',
             markeredgecolor='blue')

    #plot a vertical line where x = xmin
    plt.axvline(xmin, color='black', linewidth=1, linestyle='dotted',
                zorder=-2)

    #plot a horizontal line where y = ymin
    plt.axhline(ymin, color='black', linewidth=1, linestyle='dotted',
                zorder=-2)

    #place text for xmin (complicated...not really needed)
    transform = mpl.transforms.blended_transform_factory(ax.transData,
                                                         ax.transAxes)
    bbox_prop = dict(color='white')
    plt.text(xmin, 0.95, '%.5f' % xmin, transform=transform, ha='center',
             va='top', rotation=270, bbox=bbox_prop)

    #place text for ymin (complicated...not really needed)
    transform = mpl.transforms.blended_transform_factory(ax.transAxes,
                                                         ax.transData)
    plt.text(0.05, ymin, '%.5e' % ymin, transform=transform, ha='left',
             va='center', bbox=bbox_prop)

    #zoom out (complicated...not really needed)
    transform = lambda x,y : ax.transData.inverted().transform(
                                 ax.transAxes.transform((x,y)))
    factor = 0.1
    xmin, ymin = transform(0 - factor, 0 - factor)
    xmax, ymax = transform(1 + factor, 1 + factor)
    plt.xlim(xmin, xmax)
    plt.ylim(ymin, ymax)

    #create a set of x-values to evaluate the fit function on
    xlow, xhigh = plt.xlim()
    xspace = np.linspace(xlow, xhigh, 100)

    #plot fit function
    plt.plot(xspace, fit_function(xspace), color='blue', linestyle='solid',
             zorder=-1)

    #label x-axis
    plt.xlabel(xlabel)

    #label y-axis
    plt.ylabel(ylabel)

    #remove whitespace around plot
    plt.tight_layout()

if __name__ == '__main__':
    opts, parser = get_arguments()

    #load data
    xdata, ydata = load_data(opts.input, xcol=opts.xcol, ycol=opts.ycol)

    #fit data
    fit, func = fit_data(xdata, ydata, opts.degree)

    #find minumum
    xmin, ymin = find_minimum(func, x0=opts.x0)

    #plot result
    plot_result(xdata, ydata, func, xmin, ymin, opts.xlabel, opts.ylabel)

    #save results
    if not opts.output is None:
        oname = opts.output + '.fit'
        print 'saved %s' % oname
        print_result(oname, fit, xmin, ymin)

        oname = opts.output + '.pdf'
        print 'saved %s' % oname
        plt.savefig(oname)

        print ''

    #print result to the screen
    for i, coeff in enumerate(fit):
        print '%-8s = % .8e' % ('c%d' % i, coeff)
    print '%-8s = % .8e' % ('xmin', xmin)
    print '%-8s = % .8e' % ('ymin', ymin)

    #show the result
    if not opts.hide:
        plt.show()

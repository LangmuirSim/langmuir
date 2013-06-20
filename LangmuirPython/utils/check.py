# -*- coding: utf-8 -*-
import pandas as pd
import numpy as np
import langmuir
import argparse
import os

try:
    import quantities as _units
    cfactor = float((_units.e / _units.ps).rescale(_units.nA))
except:
    cfactor = 160.2176487

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.add_argument(dest='dat', nargs='?', default=None, help='dat file')
    opts = parser.parse_args(args)
    if opts.dat is None:
        opts.dat = langmuir.find.dat(os.getcwd(), at_least_one=True)
    return opts

def fluxinfo(data, flux):
    result = {}
    result['a'] = data[flux + ':attempt']
    result['s'] = data[flux + ':success']
    result['t'] = data['simulation:time']

    if result['a'] > 0:
        result['p'] = result['s'] / result['a'].astype(float) * 100.0
    else:
        result['p'] = 0.0

    if result['t'] == 0:
        result['r'] = 0.0
    else:
        result['r'] = result['s'] / result['t'].astype(float)

    result['c'] = result['r'] * cfactor

    return result

if __name__ == '__main__':
    opts = get_arguments()
    work = os.getcwd()
    data = langmuir.datfile.load_dat(opts.dat)
    last = data[-1]

    fluxes = {}
    fluxes['eDL'] = fluxinfo(last, 'eDrainL' )
    fluxes['eDR'] = fluxinfo(last, 'eDrainR' )
    fluxes['hDL'] = fluxinfo(last, 'hDrainL' )
    fluxes['hDR'] = fluxinfo(last, 'hDrainR' )
    fluxes['eSL'] = fluxinfo(last, 'eSourceL')
    fluxes['eSR'] = fluxinfo(last, 'eSourceR')
    fluxes['hSL'] = fluxinfo(last, 'hSourceL')
    fluxes['hSR'] = fluxinfo(last, 'hSourceR')
    fluxes['xS' ] = fluxinfo(last, 'xSource' )
    fluxes['xD' ] = fluxinfo(last, 'xDrain'  )

    frame = {}
    frame['attempt'] = {key : flux['a'] for key, flux in fluxes.iteritems()}
    frame['success'] = {key : flux['s'] for key, flux in fluxes.iteritems()}
    frame['prob'   ] = {key : flux['p'] for key, flux in fluxes.iteritems()}
    frame['rate'   ] = {key : flux['r'] for key, flux in fluxes.iteritems()}
    frame['current'] = {key : flux['c'] for key, flux in fluxes.iteritems()}
    columns = ['attempt', 'success', 'prob', 'rate', 'current']

    for col in columns:
        if not col in ['prob', 'attempt']:
            frame[col]['eL'] = frame[col]['eSL'] - frame[col]['eDL']
            frame[col]['eR'] = frame[col]['eSR'] - frame[col]['eDR']
            frame[col]['eI'] = frame[col]['eSL'] + frame[col]['eSR'] + \
                frame[col]['xS']
            frame[col]['eO'] = frame[col]['eDL'] + frame[col]['eDR'] + \
                frame[col]['xD']
            frame[col]['de'] = frame[col]['eI' ] - frame[col]['eO' ]
            frame[col]['hL'] = frame[col]['hSL'] - frame[col]['hDL']
            frame[col]['hR'] = frame[col]['hSR'] - frame[col]['hDR']
            frame[col]['hI'] = frame[col]['hSL'] + frame[col]['hSR'] + \
                frame[col]['xS']
            frame[col]['hO'] = frame[col]['hDL'] + frame[col]['hDR'] + \
                frame[col]['xD']
            frame[col]['dh'] = frame[col]['hI' ] - frame[col]['hO' ]
            frame[col]['cI'] = frame[col]['eI' ] + frame[col]['hI' ]
            frame[col]['cO'] = frame[col]['eO' ] + frame[col]['hO' ]
            frame[col]['dc'] = frame[col]['cI' ] - frame[col]['cO' ]
            frame[col]['pe'] = frame[col]['eO'] / (frame[col]['eI'] * 0.01)
            frame[col]['ph'] = frame[col]['hO'] / (frame[col]['hI'] * 0.01)
            frame[col]['pc'] = frame[col]['cO'] / (frame[col]['cI'] * 0.01)

            frame[col]['px'] = frame[col]['xD'] / (frame[col]['cI'] * 0.01)

    frame = pd.DataFrame(frame)
    frame = frame.reindex(columns=columns)
    frame = frame.fillna(value=np.nan)

    e = last['electron:count']
    h = last['hole:count']
    t = last['simulation:time']
    r = last['real:time']
    dt = np.gradient(data['simulation:time'][-100:])
    dr = np.gradient(data['real:time'][-100:])
    drdt = dr/(dt)
    drdt_avg = np.average(drdt)
    drdt_std = np.std(drdt)

    def float_format(x):
        if np.isnan(x):
            return ''
        return '%.5g' % x

    kwargs = dict(float_format = float_format, col_space=12)

    rows = []
    rows.extend(['eSL', 'eSR', 'eDL', 'eDR', 'eL', 'eR', ''])
    rows.extend(['hSL', 'hSR', 'hDL', 'hDR', 'hL', 'hR', ''])
    rows.extend(['xS', 'xD', ''])
    rows.extend(['eI', 'hI', 'cI', ''])
    rows.extend(['eO', 'hO', 'cO', ''])
    rows.extend(['de', 'dh', 'dc', ''])
    rows.extend(['pe', 'ph', 'pc', 'px', ''])
    print frame.ix[rows].to_string(**kwargs)

    print '%-8s %8d' % ('elec', e)
    print '%-8s %8d' % ('hole', h)
    print '%-8s %8d' % ('carr', e + h)
    print ''
    print '%-8s %8d' % ('step', t)
    print ''
    print '%-8s %12.3f s' % ('time', r / (1000.0))
    print '%-8s %12.3f min' % ('', r / (1000.0 * 60))
    print '%-8s %12.3f hr' % ('', r / (1000.0 * 60 * 60))
    print ''
    print '%-8s %12.3f ms/step' % ('rate avg', drdt_avg)
    print '%-8s %12.3f ms/step' % ('rate std', drdt_std)

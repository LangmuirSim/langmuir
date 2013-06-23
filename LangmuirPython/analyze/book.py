# -*- coding: utf-8 -*-
"""
Created on Fri Apr 19 15:00:57 2013

@author: adam
"""

import matplotlib.pyplot as plt
import pandas as pd
import itertools
import langmuir
import StringIO
import shutil
import os

data = langmuir.analyze.load_pkl('combined.pkl.gz')
data = langmuir.analyze.calculate(data).fillna(value=0)

work = os.getcwd()
if os.path.exists('plots'):
    shutil.rmtree('plots')
os.mkdir('plots')
os.chdir('plots')

x_col = 'simulation:time'
xmin, xmax = data[x_col].min(), data[x_col].max()
paths = []
colors = itertools.cycle(['r', 'b'])
for i, y_col in enumerate(data.columns):
    print '(%3d/%3d) : %s' % (i, len(data.columns), y_col)
    fig, ax1 = plt.subplots(1, 1)
    for key, group in pd.groupby(data, by='part'):
        x = group[x_col]
        y = group[y_col]
        plt.plot(x, y, '-', color=colors.next())
    plt.xlabel(x_col)
    plt.ylabel(y_col)
    langmuir.plot.zoom(ax1)
    oname = '%s.pdf' % y_col
    langmuir.plot.save(oname)
    paths.append(oname)

stream = StringIO.StringIO()
print >> stream, r'\documentclass{article}'
print >> stream, r'\usepackage{graphicx}'
print >> stream, r'\usepackage{longtable}'
print >> stream, r'\usepackage{tabu}'
print >> stream, r'\usepackage[margin=1cm]{geometry}'
print >> stream, r'\begin{document}'
print >> stream, r'\pagestyle{empty}'
print >> stream, r'\begin{center}'
print >> stream, r'\begin{longtabu} to \textwidth {X[1rb] X[1rb]}'
i = 0
while i < len(paths):
    col_0 = r'\includegraphics[width=0.4\textwidth]{%s}' % paths[i + 0]
    col_1 = r''
    if i < len(paths) - 1:    
        col_1 = r'\includegraphics[width=0.4\textwidth]{%s}' % paths[i + 1]
    print >> stream, r'%s & %s \\' % (col_0, col_1)
    i += 2
print >> stream, r'\end{longtabu}'
print >> stream, r'\end{center}'
print >> stream, r'\end{document}'
print stream.getvalue()

handle = open('plots.tex', 'w')
handle.write(stream.getvalue())
handle.close()

os.system('latexmk -pdf plots.tex')

os.chdir(work)
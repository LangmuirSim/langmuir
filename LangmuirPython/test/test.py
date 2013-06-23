# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import scipy.interpolate
import numpy as np
import langmuir
import os

if __name__ == '__main__':
    work = os.getcwd()

    x1 = np.linspace(-10, 10, 15)
    y1 = 0.25 * (x1 - 3)**3 + 2 - (0.5 * x1 + 2)**2 + 5 * x1
    e1 = np.random.normal(0, 5, x1.size)
    plt.errorbar(x1, y1, yerr=e1, color='r', marker='o', ls='-')


    f = langmuir.fit.FitBarycentric(x1, y1, e1)
    f.plot(color='b')

    #s1 = scipy.interpolate.lagrange(x1, y1)
    #s1 = scipy.interpolate.UnivariateSpline(x1, y1, w=e1, k=5)
    #s1 = scipy.interpolate.barycentric_interpolate(x1, y1)
    #x2 = np.linspace(-10, 10, 100)
    #y2 = s1(x2)

    #y2 = scipy.interpolate.barycentric_interpolate(x1, y1, x2)



    #plt.plot(x2, y2, 'b-', lw=1)

    plt.xlim(np.amin(x1), np.amax(x1))
    plt.ylim(np.amin(y1), np.amax(y1))
    plt.show()
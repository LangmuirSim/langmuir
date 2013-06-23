# -*- coding: utf-8 -*-
"""
.. module:: append
    :platform: Unix
    :synopsis: A module to append to langmuir simulations.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import argparse
import langmuir
import shutil
import os

def get_arguments(args=None):
    """
    Get command line arguments using :py:mod:`argparse`.

    :param args: override command line arguments
    :type args: list

    returns: :py:class:`argparse.Namespace`, option object
    """
    parser = argparse.ArgumentParser()
    parser.description  = 'Append working values to a simulation system.  '
    parser.description += 'Parameters are updated from a template input file.'
    parser.add_argument(dest='system', help='system directory')
    parser.add_argument('--work', type=str, default=os.getcwd(),
                        help='working directory')
    parser.add_argument('--template', default=None, type=str,
        help='template file to update parametrs', metavar='template.inp')
    opts = parser.parse_args(args)

    os.chdir(opts.work)
    if opts.template is None:
        if os.path.exists('./template.inp'):
            opts.parm = langmuir.parameters.load('./template.inp')
        else:
            raise RuntimeError('there is no template file')
            opts.parm = langmuir.parameters.Parameters()
            opts.parm['voltage.right'] = [
                -160.0, -170.0, -180.0, -190.0, -200.0]
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    path = os.path.basename(opts.system.rstrip(os.sep))
    path = os.path.abspath(os.path.join('./new', path))
    if os.path.exists(path):
        shutil.rmtree(path)
    os.makedirs(path)

    parm = langmuir.parameters.Parameters()
    if opts.template:
        parm.load(opts.template)
    else:
        parm['voltage.right'] = [-160.0, -170.0, -180.0, -190.0, -200.0]

    sims = []
    runs = langmuir.find.runs(opts.system, 1, 1, 1)
    for i, run in enumerate(runs):
        print i, os.path.relpath(run, work)
        new = os.path.basename(run.rstrip(os.sep))
        new = os.path.join(path, new)
        os.mkdir(new)
        os.chdir(new)
        chk = langmuir.find.chks(run, 'out*', 1, 1, 1)[-1]
        chk = langmuir.checkpoint.load(chk)
        chk.reset()
        chk.parameters.update(opts.parm)
        chk.save('sim.inp')
        sims.append(new)
        os.chdir(work)

    os.chdir('new')
    inps = langmuir.find.inps(os.getcwd(), recursive=True)
    inps = [os.path.dirname(i) for i in inps]
    command = \
    r'lscan --fresh 500000 --equil 500000 --print 1000 --stub ap$1. --mode gen'
    langmuir.common.command_script(inps, name='scan', command=command)
    os.chdir(work)

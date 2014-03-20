# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import argparse
import shutil
import os
import re
desc = """
Create simulations from images.
"""

def get_arguments(args=None):
    """
    Get command line arguments.

    :param args: list of argument strings
    """
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument('--template', default='template.inp', type=str,
                        metavar='str', help='template input file')
    options = parser.parse_args(args)

    try:
        options.template = lm.checkpoint.load(options.template)
    except IOError as e:
        if not options.template == 'template.inp':
            raise e
        options.template = None

    return options

default_format = "{name}/run.0"

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()
    pngs = lm.find.pngs(work, r=True)

    def sort_by(x):
        """Sort by path.

        :param x: path
        """
        x = os.path.splitext(os.path.basename(x))[0]
        x = x.split('-')
        for i, val in enumerate(x):
            try:
                x[i] = int(val)
            except ValueError:
                x[i] = val
        return x

    pngs.sort(key=sort_by)

    sims = os.path.join(work, 'sims')
    if os.path.exists(sims):
        shutil.rmtree(sims)
    os.mkdir(sims)

    paths = []
    for png in pngs:
        relpath = os.path.relpath(png, work)
        stub, ext = os.path.splitext(relpath)

        chk = lm.checkpoint.CheckPoint.from_image(png)

        try:
            chk.update(opts.template)
        except TypeError:
            pass

        chk.fix_traps()

        path = os.path.join(sims, default_format.format(name=stub))
        print path

        if os.path.exists(path):
            shutil.rmtree(path)

        os.makedirs(path)
        os.chdir(path)
        chk.save('sim.inp')
        os.chdir(work)

        paths.append(path)

    paths.sort(key=sort_by)

    os.chdir(sims)
    lm.common.command_script(paths, name='submit', stub='{dirname}', stub_func=lambda x : re.sub(os.sep, '_', x))
    os.chdir(work)
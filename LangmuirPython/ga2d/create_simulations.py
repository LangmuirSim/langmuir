# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import argparse
import shutil
import os

desc = """
Create simulations from images.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument('--template', default='template.inp', type=str,
                        metavar='str', help='template input file')
    opts = parser.parse_args(args)

    try:
        opts.template = lm.checkpoint.load(opts.template)
    except IOError as e:
        if not opts.template == 'template.inp':
            raise e
        opts.template = None

    return opts

dfmt = "{name}_{chk[grid.x]}x{chk[grid.y]}x{chk[grid.z]}"

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()
    pngs = lm.find.pngs(work)

    sims = os.path.join(work, 'simulations')
    if os.path.exists(sims):
        shutil.rmtree(sims)
    os.mkdir(sims)

    paths = []
    for png in pngs:
        root, base = os.path.dirname(png), os.path.basename(png)
        stub, ext = os.path.splitext(base)

        chk = lm.checkpoint.CheckPoint.from_image(png)

        try:
            chk.update(opts.template)
        except TypeError:
            pass

        chk.fix_traps()

        path = os.path.join(sims, dfmt.format(name=stub, chk=chk))
        print path

        if os.path.exists(path):
            shutil.rmtree(path)

        os.mkdir(path)
        os.chdir(path)
        chk.save('sim.inp')
        os.chdir(work)

        paths.append(path)

    os.chdir(sims)
    lm.common.command_script(paths, name='submit')
    os.chdir(work)
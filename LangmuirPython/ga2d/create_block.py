from subprocess import check_call
import langmuir as lm
import argparse
import shutil
import sys
import os

parser = argparse.ArgumentParser()
parser.add_argument('--left', type=int, default=0)
parser.add_argument('--right', type=int, default=1)
parser.add_argument('--reverse', action='store_true')
opts = parser.parse_args()

if opts.reverse:
    L = opts.left
    R = opts.right
    opts.left = R
    opts.right = L

work = os.getcwd()
script_dir = r'/home/adam/opt/src/langmuir/LangmuirPython/ga2d/generators'
script_dir = os.path.relpath(script_dir, work)

script = r'block.py'

script = os.path.join(script_dir, script)

command = 'python {script} --left {left} --right {right} {png} {output}'

pngs = lm.find.pngs(work, r=True)
pngs = [p for p in pngs if os.path.dirname(p) == work]

out_path = os.path.join(work,
    'blocked_L{L}_R{R}'.format(L=opts.left, R=opts.right))
out_path = os.path.relpath(out_path, work)
if os.path.exists(out_path):
    shutil.rmtree(out_path)
os.mkdir(out_path)

for png in pngs:
    png = os.path.relpath(png, work)
    to_call = command.format(script=script, left=opts.left, right=opts.right,
        png=png, output=out_path)
    print to_call
    check_call(to_call.split())

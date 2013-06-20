import langmuir
import argparse

parser = argparse.ArgumentParser()
parser.description = "Reset a checkpoint file to be more like a new simulation."
parser.add_argument(dest='iname', default='sim.inp', nargs='?', help='input file name')
parser.add_argument(dest='oname', default=None, nargs='?', help='output file name')
parser.add_argument('--keep', default=False, action='store_true', help='keep carriers')
opts = parser.parse_args()
if not opts.oname:
    opts.oname = opts.iname

chk = langmuir.checkpoint.load(opts.iname)
chk.reset(opts.keep, opts.keep)
chk.save(opts.oname)

print chk.parameters
print 'reset', opts.iname

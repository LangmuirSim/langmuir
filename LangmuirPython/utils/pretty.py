from subprocess import check_call
import langmuir
import argparse
import os

parser = argparse.ArgumentParser()
parser.description = "format checkpoint file to be pretty"
parser.add_argument(dest='ifile', default='out.chk',
        help='input checkpoint file name')
parser.add_argument(dest='ofile', default=None,
        help='output checkpoint file name')
opts = parser.parse_args()

if opts.ofile is None:
    opts.ofile = opts.ifile

try:
    stub, ext = os.path.splitext(opts.ifile)
    if ext == '.gz':
        check_call(['gunzip', '-v', opts.ifile])
        opts.ifile = stub
except:
    pass

chk = langmuir.checkpoint.load(opts.ifile)
chk.fix_traps()
chk.save(opts.ofile)

print 'saved %s' % opts.ofile

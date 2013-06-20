import langmuir
import argparse
import os

work = os.getcwd()
parser = argparse.ArgumentParser()
parser.description = "Search for input files and make a submit script"
parser.add_argument('--stub', default='run', type=str, help='job name stub')
parser.add_argument('--name', default='submit', type=str, help='submit script name')
opts = parser.parse_args()

inps = langmuir.find.inps(work, recursive=True)
inps = [langmuir.find.Part(os.path.dirname(inp)) for inp in inps]
print 'found %d inps!' % len(inps)

inps = [inp.work for inp in inps if not inp.dat and not inp.chk]
print 'using %d inps!' % len(inps)

if inps:
    print 'saved script : "%s"' % opts.name
    langmuir.common.command_script(inps, name=opts.name,
        stub=opts.stub)
else:
    print 'nothing to do...'

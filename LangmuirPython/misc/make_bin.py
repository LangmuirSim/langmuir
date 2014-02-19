import shutil
import os

work = os.getcwd()
bdir = os.path.join(work, 'bin')

assert os.path.exists(os.path.join(work, 'langmuir'))
assert os.path.exists(os.path.join(work, 'analyze'))
assert os.path.exists(os.path.join(work, 'utils'))

def create_script(oname, path):
    path = os.path.abspath(path)
    with open(oname, 'wb') as handle:
        print >> handle, r'#!/bin/bash'
        print >> handle, r'python %s $*' % path
    os.system('chmod +x %s' % oname)

if os.path.exists(bdir):
    shutil.rmtree(bdir)
os.mkdir(bdir)
os.chdir(bdir)

def create_scripts(path):
    root, dirs, files = os.walk(os.path.join(work, path)).next()
    for f in files:
        stub, ext = os.path.splitext(f)
        if ext == '.py':
            f = os.path.join(root, f)
            print f
            create_script('l%s' % stub, f)

create_scripts('utils')
create_scripts('analyze')
create_scripts('surface')

print ''
print '*' * 80
print 'UPDATE YOUR PATH: PATH=$PATH:%s' % bdir
print '*' * 80
print ''

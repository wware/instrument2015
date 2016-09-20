#!/usr/bin/env python

# http://pyyaml.org/wiki/PyYAMLDocumentation
# https://en.wikipedia.org/wiki/YAML#Sample_document
from yaml import load, dump
import pprint
import re

pieces = load(open('pieces.yaml'))

def hack_constants(consts):
    retval = ''
    prereqs = {}
    r = re.compile('[_a-zA-Z]+')
    for k,v in consts.items():
        prereqs[k] = []
        z = v
        while True:
            m = r.search(str(z))
            if m is None:
                break
            y = z[m.start():m.end()]
            if y not in ('sin', 'cos', 'tan', 'asin', 'acos', 'atan'):
                prereqs[k].append(y)
            z = z[m.end():]
    while prereqs:
        a = []
        for k in prereqs.keys():
            if not prereqs[k]:
                a.append(k)
        for k1 in a:
            for k in prereqs.keys():
                try:
                    prereqs[k].remove(k1)
                except ValueError:
                    pass
            retval += '{0} = {1};\n'.format(k1, consts[k1])
            del prereqs[k1]
    return retval


def hack_structure(struc):
    if isinstance(struc, list):
        return ''.join([hack_structure(x) for x in struc])
    assert isinstance(struc, dict)
    assert len(struc.keys()) == 1
    key = struc.keys()[0]
    if key in ('intersection', 'difference'):
        r = key + '(){'
        r += ''.join([hack_structure(x)+';' for x in struc[key]])
        r += '}'
        return r
    elif key in ('translate', 'rotate', 'mirror'):
        lst = struc[key]
        v = lst.pop(0)
        v = "[{0},{1},{2}]".format(v['x'], v['y'], v['z'])
        r = key + '({0}){{'.format(v)
        r += ''.join([hack_structure(x)+';' for x in struc[key]])
        r += '}'
        return r
    elif key == 'cube':
        v = struc[key]
        v = "[{0},{1},{2}]".format(v['x'], v['y'], v['z'])
        return 'cube({0})'.format(v)
    elif key == 'cylinder':
        args = struc[key]
        args = ','.join('{0}={1}'.format(k,v) for k, v in args.items())
        return 'cylinder({0})'.format(args)
    else:
        args = struc[key]
        args = ','.join('{0}={1}'.format(k,v) for k, v in args.items())
        return key + '({0})'.format(args)

def module(name, pieces):
    pieces = pieces[name]
    r = 'module ' + name + '('
    r += ','.join(pieces.get('args', []))
    r += '){' + hack_structure(pieces['structure']) + ';}'
    return r


print hack_constants(pieces['constants'])

for thing in pieces.keys():
    if 'structure' in pieces[thing]:
        print module(thing, pieces)

print 'halfpipe();'
# print 'machinescrew(2*INCH);'
# print 'plywood();'
print 'panel();'

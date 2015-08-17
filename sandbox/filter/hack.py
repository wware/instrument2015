import math
import os
import pprint
import sys

SAMP_FREQ = 40000
DT = 1. / SAMP_FREQ

class Stats:
    def __init__(self):
        self.n = 0
        self.sum = self.sumsq = 0.
        self.min = self.max = None

    def __repr__(self):
        return "{0}+/-{1}, [{2},{3}]".format(
            self.mean, self.deviation, self.min, self.max
        )

    def add(self, x):
        self.max = max(self.max, x)
        if self.min is None:
            self.min = x
        else:
            self.min = min(x, self.min)
        self.n += 1
        self.sum += x
        self.sumsq += x * x

    def _mean(self):
        return self.sum / self.n
    mean = property(_mean)

    def _deviation(self):
        m = self.mean
        sigmasq = self.sumsq - m**2
        return sigmasq ** 0.5
    deviation = property(_deviation)

def filter(f0, q):
    stats = [Stats(), Stats(), Stats(), Stats()]
    k = 1. / q
    w0 = 2 * math.pi * f0
    integrators = [0., 0.]
    def iterate(x):
        stats[0].add(x)
        u = x - 2 * k * integrators[0] - integrators[1]
        result = (u, integrators[0], integrators[1])
        integrators[1] += w0 * DT * integrators[0]
        integrators[0] += w0 * DT * u
        stats[1].add(u)
        stats[2].add(integrators[0])
        stats[3].add(integrators[1])
        return result
    def response(f):
        s = 2j * math.pi * f
        D = s**2 + 2 * k * w0 * s + w0**2
        return (s**2 / D, w0 * s / D, w0**2 / D)
    return (iterate, response, stats)


# http://gnuplot.respawned.com/
"""
# Scale font and line width (dpi) by changing the size! It will always display stretched.
set terminal svg size 400,300 enhanced fname 'arial'  fsize 10 butt solid
set output 'out.svg'
# Key means label...
set key inside bottom right
set xlabel 'Frequency (Hz)'
set title 'Second order filter'
plot  "data.txt" using 1:2 title 'HP' with lines, "data.txt" using 1:3 title 'BP' with lines, "data.txt" using 1:4 title 'LP' with lines
"""


if False:
    _, R, _ = filter(100, 2)

    for f in range(200):
        hp, bp, lp = R(f)
        print f, abs(hp), abs(bp), abs(lp)

else:
    """
    Demonstration that a discrete 2nd-order time filter can work as expected.
    I don't need Z transforms after all.
    """
    I, _, stats = filter(100 * 1.6, 2)   # why the factor of 1.6 in frequency??

    outf = open('foo.gp', 'w')
    for i in range(SAMP_FREQ):
        f = 40 + (i * 120. / SAMP_FREQ)
        t = DT * i
        x = math.sin(2 * math.pi * f * t)
        hp, bp, lp = I(x)
        if i & 7 == 0:
            print >> outf, f, hp, bp, lp
    outf.close()
    pprint.pprint(stats)

    os.system("echo \"set term png; set output 'output.png';"
    " plot 'foo.gp' using 1:2 title 'HP' with lines, "
    "'foo.gp' using 1:3 title 'BP' with lines, "
    "'foo.gp' using 1:4 title 'LP' with lines\" | gnuplot")

import math

SAMP_FREQ = 40000
DT = 1. / SAMP_FREQ

def filter(f0, q):
    k = 1. / q
    w0 = 2 * math.pi * f0
    integrators = [0., 0.]
    def iterate(x):
        u = x - 2 * k * integrators[0] - integrators[1]
        result = (u, integrators[0], integrators[1])
        integrators[1] += w0 * DT * integrators[0]
        integrators[0] += w0 * DT * u
        return result
    def response(f):
        s = 2j * math.pi * f
        D = s**2 + 2 * k * w0 * s + w0**2
        return (s**2 / D, w0 * s / D, w0**2 / D)
    return (iterate, response)


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
    _, R = filter(100, 2)

    for f in range(200):
        hp, bp, lp = R(f)
        print f, abs(hp), abs(bp), abs(lp)

else:
    """
    Demonstration that a discrete 2nd-order time filter can work as expected.
    I don't need Z transforms after all.
    """
    I, _ = filter(100 * 1.6, 2)   # why the factor of 1.6 in frequency??

    for i in range(SAMP_FREQ):
        f = 40 + (i * 120. / SAMP_FREQ)
        t = DT * i
        x = math.sin(2 * math.pi * f * t)
        hp, bp, lp = I(x)
        if i & 7 == 0:
            print f, hp, bp, lp

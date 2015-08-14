import instrument

assert instrument.Key is not None
assert instrument.Queue is not None

q = instrument.Queue()
assert q.write(4) == 0

# this should succeed (0) with result 4
x = q.read()
assert x == [0, 4]

# this should fail (1)
x = q.read()
assert x[0] == 1

# fill up the queue, remember it holds only 1023 samples
for i in range(1023):
    assert q.write(i) == 0, i

# queue is now full, expect failure
assert q.write(1023) == 1

# verify queue contents
for i in range(1023):
    r, j = q.read()
    assert r == 0, r
    assert j == i, (j, i)

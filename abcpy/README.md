Introduction
------------

ABCpy is a Python library to parse [ABC music notation](http://abcnotation.com/).
It is being developed by [Robert Adams](mailto:d.robert.adams@gmail.com) for
personal use, and is not intended to support every feature of the ABC standard.
It supports most of the notation found in Irish/Scottish/Canadian/American
traditional tunes. All of the documentation is found inline, or check out the
unit test file, test.py.

Current Limitations/Assumptions
-------------------------------

* Each input file has exactly one tune.
* Each tune has exactly one voice.
* Does not handle triplets.
* Does not handle grace notes.

#!/usr/bin/env python
# encoding: utf-8
"""
sample.py

"""

import abcpy
import unittest

class ConstructorTests(unittest.TestCase):
		
	def test_init_no_file(self):
		tune = abcpy.Tune()
		
	def test_init_with_good_file(self):
		tune = abcpy.Tune('test.abc')
		
	def test_init_with_bad_file(self):
		with self.assertRaises(IOError):
			tune = abcpy.Tune('xyzzy')
	
##############################################################################
class TuneTester(unittest.TestCase):
	def setUp(self):
		self.tune = abcpy.Tune()
		self.tune.parse("""
		% The test tune for the unit tests for abcpy.py
		X:1
		T:Title % With a comment
		T:Another title, this one should be ignored.
		O: the origin
		M:4/4
		L: 1/4
		K: D
		(3e/2fg | "Em"g/2e/2f/2d/2 "X"e/2c/2d/2B/2 | "D"A/2B/2A/2F/2 DF/2D/2 |    \
		"Em"E/2F/2G/2A/2 Be/2f/2 | "C"g/2f/2e/2d/2 "Bm"Ae/2f/2 |
		"Em"g/2e/2f/2d/2 e/2c/2d/2B/2|"D"A/2B/2A/2F/2 DF/2D/2|"Em"E/2F/2G/2A/2 BA/2G/2\
		|"Bm"F/2D/2G/2F/2 "Em"E::
		A|"Em"Be/2B/2 g/2B/2e/2B/2|"Em"Be/2B/2 g/2B/2e/2B/2|"D"df/2d/2 g/2d/2f/2d/2|\
		"D"df/2d/2 g/2d/2f/2d/2
		 | "Em"Be/2B/2 g/2B/2e/2B/2 | "Em"Be/2B/2 g/2B/2e/2B/2|"Bm"f3/2g/2 f/2e/2d/2c/2|\
		"Bm"B>cd/2d/2 "Em"e:|
		""")
		
	def test_x_field(self):
		self.assertEquals(self.tune.field('X'), '1')

	def test_find_info_key(self):
		self.assertEquals(self.tune.field('K'), 'D')
				
	def test_find_info_title(self):
		self.assertEquals(self.tune.field('T'), 'Title')
		
	def test_find_info_meter(self):
		self.assertEquals(self.tune.field('M'), '4/4')	
		
	def test_find_info_note_length(self):
		self.assertEquals(self.tune.field('L'), '1/4')	
			
	def test_read_tune_body(self):
		self.assertEquals(len(self.tune.text), 4)

	def test_find_measures(self):
		self.assertEquals(len(self.tune.line[0].measure), 5)
		self.assertEquals(len(self.tune.line[1].measure), 4)
		self.assertEquals(len(self.tune.line[2].measure), 5)
		self.assertEquals(len(self.tune.line[3].measure), 4)
		
	def test_chord_finding(self):
		self.assertEquals(str(self.tune.line[0].measure[1].item[0]), "Em")
		self.assertEquals(str(self.tune.line[3].measure[3].item[0]), "Bm")
		
	def test_note_finding(self):
		self.assertEquals(str(self.tune.line[0].measure[0].item[0]), "e/2")
		self.assertEquals(str(self.tune.line[3].measure[2].item[1]), "f3/2")
		self.assertEquals(str(self.tune.line[3].measure[3].item[1]), "B>")
		
	def test_multi_line_beat(self):
		self.assertEquals(self.tune.line[3].measure[0].item[0].beat, 1)
		self.assertEquals(self.tune.line[3].measure[3].item[6].beat, 4)

##############################################################################
class LineTester(unittest.TestCase):
	def setUp(self):
		self.l = abcpy.Line()
		
	def test_garbage_line(self):
		self.l.parse("!@#$")
		self.assertEquals(len(self.l.measure), 1)
		
	def test_simple_line(self):
		self.l.parse('ABCD DEFG | ABCD DEFG | ABCD DEFG')
		self.assertEquals(len(self.l.measure), 3)

	def test_empty_measures(self):
		self.l.parse(' | ABCD DEFG | ABCD DEFG')
		self.assertEquals(len(self.l.measure), 2)		
		self.l.parse(' ABCD DEFG | | ABCD DEFG')
		self.assertEquals(len(self.l.measure), 2)
		self.l.parse(' ABCD DEFG | ABCD DEFG |')
		self.assertEquals(len(self.l.measure), 2)
	
	def test_multi_measure_beats(self):
		self.l.parse('A B C | D E F | G a b')
		self.assertEquals(self.l.measure[1].item[1].beat, 2)
		self.assertEquals(self.l.measure[2].item[2].beat, 3)
		
	def test_beats_with_repeats(self):
		self.l.parse("A B C D |[1 E F G a :|[2 b c d e")
		self.assertEquals(self.l.measure[1].item[0].beat, 1)
		self.assertEquals(self.l.measure[2].item[0].beat, 1)
						
##############################################################################
class MeasureTester(unittest.TestCase):
	def setUp(self):
		self.m = abcpy.Measure()
		
	def test_garbage_measure(self):
		self.m.parse("!@#$")
		self.assertEquals(len(self.m.item), 0)
		
	def test_simple_measure(self):
		self.m.parse("ABCD DEFG")
		self.assertEquals(str(self.m.item[0]), "A")
		self.assertEquals(str(self.m.item[7]), "G")
		
	def test_complex_measure(self):
		self.m.parse('"C1"^A,3/2 B/2 C/4 D8 ^D ^E, ^F> "C2" ^G,')
		self.assertEquals(str(self.m.item[0]), "C1")
		self.assertEquals(self.m.item[1].note, "A")
		self.assertEquals(str(self.m.item[8]), "C2")
		self.assertEquals(self.m.item[9].note, "G")		
	
	def test_simple_beat(self):
		self.m.parse("A B C D")
		self.assertEquals(self.m.item[0].beat, 1)
		self.assertEquals(self.m.item[1].beat, 2)
		self.assertEquals(self.m.item[2].beat, 3)
		self.assertEquals(self.m.item[3].beat, 4)
		
	def test_beat_with_chords(self):
		self.m.parse('"A" A B "C" C D')
		self.assertEquals(self.m.item[0].beat, 1)
		self.assertEquals(self.m.item[1].beat, 1)
		self.assertEquals(self.m.item[2].beat, 2)
		self.assertEquals(self.m.item[3].beat, 3)
		self.assertEquals(self.m.item[4].beat, 3)
		self.assertEquals(self.m.item[5].beat, 4)
		
	def test_beat_with_long_notes(self):
		self.m.parse('"Em"E2 E GFE')
		self.assertEquals(self.m.item[0].beat, 1)
		self.assertEquals(self.m.item[1].beat, 1)
		self.assertEquals(self.m.item[2].beat, 3)
		self.assertEquals(self.m.item[3].beat, 4)
		self.assertEquals(self.m.item[4].beat, 5)
		self.assertEquals(self.m.item[5].beat, 6)
		
	def test_another_beat_test(self):
		self.m.parse('"B"^d2 "C"e2 "B"f2')
		self.assertEquals(self.m.item[0].beat, 1)
		self.assertEquals(self.m.item[1].beat, 1)
		self.assertEquals(self.m.item[2].beat, 3)
		self.assertEquals(self.m.item[3].beat, 3)
		self.assertEquals(self.m.item[4].beat, 5)
		self.assertEquals(self.m.item[5].beat, 5)
		
	def test_complex_beat(self):
		self.m.parse('"Em" g/2 e/2 f/2 d/2 "Em" e/2 c/2 "D" d/2 B/2')
		self.assertEquals(self.m.item[0].beat, 1) # Em chord
		self.assertEquals(self.m.item[1].beat, 1)
		self.assertEquals(self.m.item[2].beat, 1.5)
		self.assertEquals(self.m.item[3].beat, 2)
		self.assertEquals(self.m.item[4].beat, 2.5)
		self.assertEquals(self.m.item[5].beat, 3) # Em chord
		self.assertEquals(self.m.item[6].beat, 3)
		self.assertEquals(self.m.item[7].beat, 3.5)
		self.assertEquals(self.m.item[8].beat, 4) # D chord
		self.assertEquals(self.m.item[9].beat, 4)
		self.assertEquals(self.m.item[10].beat, 4.5)

	def test_repeat_numbers(self):
		self.m.parse("A B C D [1")
		self.assertEquals(self.m.repeat, 1)
		self.m.parse("A B C D [4")
		self.assertEquals(self.m.repeat, None)
		
##############################################################################
class NoteTester(unittest.TestCase):
	def setUp(self):
		self.n = abcpy.Note()
		
	def test_error_conditions(self):
		with self.assertRaises(RuntimeError):
			self.n.parse('')
		with self.assertRaises(RuntimeError):
			self.n.parse('^')
			
	def test_plain_note(self):
		self.n.parse("A")
		self.assertEquals(str(self.n), "A")
		self.assertIsNone(self.n.prefix)
		self.assertEquals(self.n.note, "A")
		self.assertIsNone(self.n.suffix)
		self.assertIsNone(self.n.length)
		
	def test_complex_note(self):
		self.n.parse("^A,3/2")
		self.assertEquals(str(self.n), "^A,3/2")
		self.assertEquals(self.n.prefix, "^")		
		self.assertEquals(self.n.note, "A")		
		self.assertEquals(self.n.suffix, ",")		
		self.assertEquals(self.n.length, "3/2")
		
	def test_note_durations(self):
		self.n.parse("A")
		self.assertEquals(self.n.duration, 1)		
		self.n.parse("A4")
		self.assertEquals(self.n.duration, 4)
		self.n.parse("A/2")
		self.assertEquals(self.n.duration, 0.5)
		self.n.parse("A1/4")
		self.assertEquals(self.n.duration, 0.25)
		self.n.parse("A3/2")
		self.assertEquals(self.n.duration, 1.5)
		
	def test_cut_note_duration(self):
		self.n.parse("A>B")
		self.assertEquals(self.n.duration, 1.5)
		self.n.parse("B")
		self.assertEquals(self.n.duration, 0.5)
		self.n.parse("A<B")
		self.assertEquals(self.n.duration, 0.5)		
		self.n.parse("B")
		self.assertEquals(self.n.duration, 1.5)												
		
##############################################################################
class ChordTester(unittest.TestCase):
	def setUp(self):
		self.c = abcpy.Chord()
		
	def test_error_conditions(self):
		with self.assertRaises(RuntimeError):
			self.c.parse('')
		with self.assertRaises(RuntimeError):
			self.c.parse('Em"')
		with self.assertRaises(RuntimeError):
			self.c.parse('"Em')
			
	def test_valid_chord(self):	
		self.c.parse('"Em"')
		self.assertTrue(str(self.c), "Em")
								
if __name__ == '__main__':
	unittest.main()
#!/usr/bin/python
# ABCpy Python Parser for ABC Music Notation Files
# Robert Adams (d.robert.adams@gmail.com)
from __future__ import division

version = '0.1'

import re
import string

# Because the '<' and '>' note duration implies a note length on the next note
# we have to keep track that we just saw a '<' or '>' so we know the length
# of the next note. So if we see 'A>B' we know that A is 1.5, and B will be
# 0.5.
_nextNoteDuration = 1

class Tune:
	"""
	Represents an entire tune with information fields and music.

	Properties
	----------
	text
		An array of the lines of the tune, as strings.
	line
		An array of the lines of the tune, as Line objects (see below).
	"""
	
	def __init__(self, filename=None):
		"""
		Creates a Tune object. If a filename is given, the file is opened and
		parsed.If an invalid filename is given, throws IOError.
		"""
		
		self._fields = {}	# information fields
		self.text = []    	# array of tune lines as strings
		self.line = []   	# array of tune lines as Line
		
		if filename:
			f = open(filename, 'Ur')
			self.parse(f.read())
			f.close()

	def field(self, field):
		"""
		Returns an information field (e.g., "T", "X"), or None if the given field
		doesn't exist.
		"""
		if field in self._fields:
			return self._fields[field]
		else:
			return None
			
	def parse(self, str):
		"""
		Parses the given input ABC string.
		"""
		lineBuffer = ''
		lines = str.split('\n')
				
		for line in lines:
			
			# Strip superfluous characters.
			line = re.sub('%.*$', '', line) # Strip comments.
			line = line.lstrip().rstrip()   # Strip whitespace.
			
			# Ignore blank lines.
			if len(line) == 0:
				continue
						
			# If the lines begins with a letter and a colon, it's an information
			# field. Extract it.
			matches = re.match('([A-Za-z]):\s*(.*)', line)
			if matches:
				self._parseInformationField(matches.group(1), matches.group(2))
			else:
				# We have a tune line.
				if line[-1] == "\\":
					# The current line ends with a \, so just store it in the buffer
					# for now.
					lineBuffer += line.rstrip("\\")
				else:
					# The current line does not end with a \, so add it to whatever
					# lines we might have seen previously and parse it.
					lineBuffer += line
					self.text.append(lineBuffer) # Store raw tune line.
					self.line.append(Line(lineBuffer))
					lineBuffer = ''
				
	def _parseInformationField(self, field, data):
		# Parses an information field. field is a letter, while data is the
		# data following the field identifier. field is converted to uppercase 
		# before storage. Only the first occurrence of the field is stored.
		field = field.upper()
		if field not in self._fields:
			self._fields[field] = data

##############################################################################
class Line(object):
	"""
	Represents one line in a tune.
	
	Properties
	----------
	text
		The raw text that was parsed.
	measure
		An array of Measure objects representing the individual measures
		within the line.
	"""
	
	def __init__(self, line=None):
		"""
		Takes a text line and parses it.
		"""
		self.text = None 	# raw text of the line
		self.measure = [] 	# array of Measures
		if line:
			self.parse(line)
		
	def parse(self, line):
		"""
		Parses a line of ABC.
		"""
		self.__init__()
		self.text = line
		
		# Split the line into measures. Measure symbols are
		# |, |], ||, [|, |:, :|, ::
		measures = re.split('\||\|]|\|\||\[\||\|:|:\||::', line)
		
		# Remove empty measures (typically at the end of lines).
		for item in measures:
			if len(item.lstrip().rstrip()) == 0:
				measures.remove(item)
			
		self.measure = [] # array of Measure objects
		for measure in measures:
			newMeasure = Measure()
			newMeasure.parse(measure)
			self.measure.append(newMeasure)
			
	def __str__(self):
		return self.text
	
##############################################################################		
class Measure(object):
	"""
	Represents one measure of a line of music.
	
	Properties
	----------
	text
		The raw text of the measure that was parsed.
	item
		An array of MusicItem objects representing the individual items (notes and
		chords) within this measure.
	repeat
		The repeat number for this measure, or None if there is no repeat.
		This only simply repeats, e.g., [1 and [2
	"""
	
	def __init__(self):
		"""
		Constructor. Builds an empty Measure object.
		"""
		self._reset()
					
	def parse(self, text):
		"""
		Parses a string of ABC into Notes and Chords.
		"""
		self._reset()
		self.text = text
		
		match = re.search('\[([12])', self.text)
		if match:
			# First or second repeat.
			self.repeat = int(match.group(1))
			self._pos += len(match.group(0))
			
		while self._pos < len(self.text):

			if self.text[self._pos].isspace():
				# Ignore whitespace.
				self._pos += 1
							
			elif self.text[self._pos] == '"':
				# Parse a chord.
				self._parseChord()

			elif self.text[self._pos] in "^=_" or self.text[self._pos].isalpha():
				# Found the start of a note.
				self._parseNote()
			
			else:		
				# Skip over anything we don't recognize.
				self._pos += 1

	def _parseChord(self):
		# Parses a chord.
		newChord = Chord()
		chordText = newChord.parse(self.text[self._pos:])
		newChord.beat = self._beat
		self._beat += newChord.duration
		self.item.append(newChord)
		self._pos += len(chordText) + 2 # add 2 to account for the double quotes

	def _parseNote(self):
		# Parses a note.		
		newNote = Note()
		noteText = newNote.parse(self.text[self._pos:])
		newNote.beat = self._beat
		self._beat += newNote.duration
		self.item.append(newNote)
		self._pos += len(noteText)
			
	def _reset(self):
		# Clears out all data.
		self.item = []		# array of Chords and Notes for this measure
		self.text = None	# raw text of the measure
		self._pos = 0 		# parsing position within the measure
		self.repeat = None	# repeat number (1 or 2)
		self._beat = 1		# current beat (while parsing)
							
	def __str__(self):
		return self.text

##############################################################################
class MusicItem(object):
	"""
	Abstract base class for "things" that appear in a line of music: 
	notes and chords.
	
	Properties
	----------
	duration
		Length of this item as a float, e.g., 0.25, 1, etc.
	beat
		The beat on which this item occurs (float). Starts at 1.
	text
		The raw text of this item.
	"""

	def __init__(self):
		# Duration of the item as a float, e.g,. 1/4, 1/8, 1/16, 2
		self.duration = 0.0 
		
		# The beat on which this item occurs: 0, 1, 2, etc.
		self.beat = 0.0
	
		# Raw text from the tune that makes up this item.
		self.text = ''
	
	def __str__(self):
		return self.text
	
##############################################################################
class Chord(MusicItem):
	"""
	Represents a chord.
	"""
	def __init__(self):
		super(Chord, self).__init__()
		
	def parse(self, str):
		"""
		Parses a chord out of the given string. Returns the raw text that 
		was parsed from str without the surrounding	double quotes.
		"""
		pos = 0
		if pos < len(str) and str[pos] == '"':
			self.text += str[pos]
			pos += 1
		else:
			raise RuntimeError('Chord does not begin with ".' + str)
			
		while pos < len(str) and str[pos] != '"':
			self.text += str[pos]
			pos += 1
			
		if pos < len(str) and str[pos] == '"':
			self.text += str[pos]
			pos += 1
		else:
			raise RuntimeError('Chord does not end with ":' + str)
			
		# Remove surrounding double quotes.
		self.text = self.text[1:-1]
		return self.text
	
##############################################################################
class Note(MusicItem):
	"""
	Represents a note.
	
	Properties
	----------
	prefix
		Optional ^, =, or _
	note
		The note character itself, A, B, etc.
	suffix
		Optional ' or ,
	length
		Optional note length, /4, 2, etc.
	"""
	def __init__(self):
		super(Note, self).__init__()
		self.prefix = None	# optional ^, =, or _
		self.note = None	# note character [A-z]
		self.suffix = None	# optional ' or ,
		self.length = None	# optional length indication
		
	def parse(self, str):
		"""
		Parses a note out of the given string. Returns the raw text that 
		was parsed from str.
		"""
		
		self.__init__()
		pos = 0		
		if pos < len(str) and str[pos] in "^=_":
			# Sharp, natural, or flat symbol.
			self.text += str[pos]
			self.prefix = str[pos]
			pos += 1

		if pos < len(str) and str[pos].isalpha():
			# Note letter.
			self.text += str[pos]
			self.note = str[pos]
			pos += 1
		else:
			raise RuntimeError('Note does not contain a character: ' + str)

		if pos < len(str) and str[pos] in "',":
			# Note raise or lower an octave.
			self.text += str[pos]
			self.suffix = str[pos]
			pos += 1

		while pos < len(str) and str[pos] in "/0123456789><":
			# Note length.
			self.text += str[pos]
			if not self.length:
				self.length = ""
			self.length += str[pos]
			pos += 1
			
		# Turn the note length(string) into a duration(float).
		global _nextNoteDuration
		if not self.length:
			self.duration = _nextNoteDuration
			_nextNoteDuration = 1
		elif self.length[0] == '<':
			self.duration = 0.5
			_nextNoteDuration = 1.5
		elif self.length[0] == '>':
			self.duration = 1.5
			_nextNoteDuration = 0.5
		else:
			if self.length[0] == '/':
				self.length = '1' + self.length
			self.duration = eval(self.length)
			_nextNoteDuration = 1
			
		return self.text

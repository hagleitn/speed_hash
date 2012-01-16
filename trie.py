import sys

class Trie:
	"""Trie data structure"""
	def __init__(self):
		self._data = {}
		self._is_word = False

	def insert(self, word):
		"""Inserts new word into dictionary"""
		if not word:
			self._is_word = True
			return
		
		head, tail = word[0], word[1:]
		
		if head in self._data:
			next_ = self._data[head]
		else:
			next_ = Trie()
			self._data[word[0]] = next_
			
		next_.insert(word[1:])
		
	def check(self, word):
		"""Checks if word is contained in dictionary"""
		if not word and self._is_word:
			return True
		
		cur = self

		for c in word:
			if not c in cur._data:
				return False
			cur = cur._data[c]

		return cur._is_word
	
	def load(self, filename):
		"""Fills trie with strings from filename. One string per line."""
		count = 0
		file = open(filename,'r')
		while True:
			line = file.readline()
			if not line:
				break
			self.insert(line.strip())

			count += 1
			if not count % 10000:
				print 'loaded: ',count
				
		file.close()

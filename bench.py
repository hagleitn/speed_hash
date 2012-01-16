import time
import sys
from trie import Trie

dictionary = Trie()
dictionary.load(sys.argv[1])

file = open(sys.argv[2], 'r')
lines = file.readlines()
file.close()

start = time.time()

for line in lines:
	b = dictionary.check(line.strip())
	if not b:
		print "Not in dict: ",line

elapsed = time.time() - start
print "Time: ", elapsed, " seconds"

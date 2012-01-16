
class Histogram:
	def __init__(self):
		self._data = {}

	def insert(self, item):
		if item in self._data:
			self._data[item] += 1
		else:
			self._data[item] = 1

	def load(self, fname, report_increment = 1000000):
		count = 0
		file = open(fname,'r')
		while True:
			line = file.readline()
			if not line:
				break
			self.insert(line.strip())

			count += 1
			if not count % report_increment:
				print 'loaded: ', count
				
		file.close()

	def print_report(self):
		keys = sorted(self._data.keys())
		for k in keys:
			print k, " ",self._data[k]

if __name__ == "__main__":
	import sys
	
	hg = Histogram()
	hg.load(sys.argv[1])
	hg.print_report()
	

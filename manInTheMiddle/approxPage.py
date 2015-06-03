import Constants
import re

reg=re.compile('^[0-1]+$')

class approxPage:
	
	def __init__(self,data,chip=-1,loc=-1):

		# Verify data size and format
		if len(data)!=Constants.PAGE_SIZE:
			raise ValueError, 'Incorrect data size'
			
		if not reg.match(data):
			raise ValueError, 'Data contains unexpected character'

		# Assign values
		self.errorString = data
		self.chip = chip # Initializing chip to 0 meaning no assignment
		self.loc = loc # Initilizing location to 0 meaning no location


	# Overload equality to check if data are within noise distance of one another
	def __eq__(self,other):
		if distance(self.errorString,other.errorString)<Constants.NOISE_DISTANCE:
			return True
		else:
			return False

# Hamming distance of two pages
def distance(self,other):
	return sum(c1 != c2 for c1, c2 in zip(self, other))
	
import Constants
from approxPage import approxPage
import shutil
import os
import sys
import random
import getopt


def main(argv):
	# print 'Start'
	# t=approxPage('10101')
	# t2=approxPage('00100')
	# t3=approxPage('11011')
	# t4=approxPage('00000')
	# add([t2,t3])
	# add([t,t2])
	# add([t,t2,t3,t4])
	# print 'Finish'
	try:
		opts, args = getopt.getopt(argv, "ha:c:", ["help", "add=","check="])
	except getopt.GetoptError:
		usage()
	for opt, arg in opts:    
		if opt in ("-h", "--help"):      
			usage()                                       
		elif opt in ("-a","--add"):              
			pages = processData(arg)
			add(pages)      
                             
		elif opt in ("-c", "--check"): 
			pages = processData(arg)
			res = match(pages)

def processData(filename):
	pages=[]
	if not os.path.exists(filename):
		print 'File not found'
		return []
	with open(filename, "r") as f:
		while True:
			c = f.read(Constants.PAGE_SIZE)
			if not c:
				break
			else:
				print 'appending ',c
				pages.append(approxPage(c))
		return pages

def usage():
	print '-a --add --> Add a fingerprint file to database'
	print '-c --check --> Check whether a fingerprint matches a chip in database'
	print '-h --help --> Help'

#Adds an array of pages to database. Assumes ordering of pages matches ordering of memory
def add(pages):
	chip = chipAdd(pages)
	print 'Created chip ', chip
	m=match(pages,chip)
	if m==[]:
		return 0
	else:
		#Need to shift the offset of other merges if the first merge cause the page to land in the middle of another one
		if m[0]['offset']==0:
			for i in range(1,len(m)):
				m[i]['offset']+=m[0]['loc']


		for i in range(0,len(m)):
			chip=chipMerge(chip,m[i]['offset'],m[i]['chip'],m[i]['loc'])
			print 'now the chip number is ',chip

	#Try to find matches in the existing database. Add data as a new chip upon failure.
	# m=match(pages)
	# if m[0]['chip']==-1:
	# 	chipAdd(pages)
	# 	return 0
	#Merge pages into the first matching chip
	# (chip,loc)=merge(pages,m[0]['offset'],m[0]['chip'],m[0]['loc'])
	# #If there is more than one chip matching, it shows link between two of the previous recorded chips. So we merge...
	# for i in range(1,len(m)):
	# 	chipMerge(chip,loc+m[i]['offset'],m[i]['chip'],m[i]['loc'])

	return 0

#Merge two chips togather starting at specific address in each
def chipMerge(chip1,loc1,chip2,loc2):
	print 'merging ',chip1,':',loc1,' to ',chip2,':',loc2
	pages=[]
	l1=int(loc1)
	l2=int(loc2)

	for i in range(0,max(l1,l2)):
		if (l1<l2):
			pages.append(pageRead(chip2,i))
		elif (l2<l1):
			pages.append(pageRead(chip1,i))

	page1=pageRead(chip1,l1)
	page2=pageRead(chip2,l2)
	#Merge two chips page by page
	while (page1 or page2):
		pages.append(pageMerge(page1,page2))
		l1+=1
		l2+=1
		page1=pageRead(chip1,l1)
		page2=pageRead(chip2,l2)
	#Remove old chips from database
	chipRemove(chip1)
	chipRemove(chip2)
	#Make a new chip using the combined pages
	newChip = chipAdd(pages)
	return newChip

def getChipNumber():
	newChip = random.randint(1,1000)
	while os.path.exists(os.path.join('database',str(newChip))):
		newChip = random.randint(1,1000) #ToDo only 1000 chips? not really efficient
	return newChip

def chipAdd(pages):
	newChip = getChipNumber()
	i=0
	for page in pages:
		page.chip=newChip
		page.loc=i
		i+=1
		pageWrite(page)
	return newChip

def chipRemove(chip):
	shutil.rmtree('database/'+str(chip))

def match(pages,newChip=0):
	#needs to care if first match is between page 0 and page >=0 of something
	#only unique chips
	res=[]
	chips=[]
	chips.append(newChip)
	for k in range(len(pages)):
		m=find(pages[k],chips)
		if m:
			if m['chip'] not in chips:
				chips.append(m['chip'])
				m['offset']=k
				res.append(m.copy())
	print 'matching res: ',res
	print 'matcing chips: ',chips
		# for i in m:
		# 	#This ensures that only first matching per chip is recorded
		# 	if i['chip']!=-1 and i['chip'] not in chips:
		# 		chips+=i['chip']
		# 		res+=i
	#Returns results if they exist. 
	return res

def pageWrite(page):


	filename=os.path.join('database',str(page.chip),(str(page.loc)+'.txt'))
	if not os.path.exists(os.path.dirname(filename)):
	    os.makedirs(os.path.dirname(filename))
	with open(filename, "w") as f:
	    f.write(page.errorString)
	return 0

def pageRead(chip,loc):
	filename=os.path.join('database',str(chip),(str(loc)+'.txt'))
	if not os.path.exists(filename):
		return []
	with open(filename, "r") as f:
		return approxPage(f.read(),chip,loc)

def find(page,ignorelist):
	chips= os.walk('database/').next()[1]
	res=[] #TODO implement
	if ignorelist:
		for x in ignorelist:
	  		chips.remove(str(x))
  	for c in chips:
  		for f in os.listdir(os.path.join('database',str(c))):
  			if f.endswith(".txt"):
  				otherPage = pageRead(str(c),f[0:-4])
  				if otherPage==page:
  					print 'matched with ',otherPage.chip,otherPage.loc
  					res={'chip':otherPage.chip,'loc':otherPage.loc}
  					return res
  			#fil = open(os.path.join('database',str(c),f),'r')
			#if distance(page.errorString==fil.read())
	return res

# def merge(pages,offset,chip,loc):
# 	if offset==0:
# 		for page in pages:
# 			p=pageRead(chip,loc)
# 			p=pageMerge(page,p,chip,loc)
# 			pageWrite(page)
# 			loc+=1
# 		return (chip,loc-1)

def pageMerge(page1,page2):
	#TODO: This doesn't actually merge. Should formalize a method to use both data to minimize noise
	if page1:
		print 'page1',page1.errorString 
		return approxPage(page1.errorString)
	elif page2:
		print 'page2',page2.errorString
		return approxPage(page2.errorString)


def rollback(number):
	return NotImplemented

if __name__ == "__main__":
    main(sys.argv[1:])
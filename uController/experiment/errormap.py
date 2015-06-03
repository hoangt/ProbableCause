import os
import sys
import time
import logging
import shutil

logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)




BST_CODE = "dram_bst"
FINGERPRINT_CODE = "dram_fingerprint"
IMGDECAY_CODE = "dram_imgdecay"
FINGERPRINT_OUTPUT='fingerprint'
IMAGEDECAY_OUTPUT = 'imagedecay'
SEGMENT_SIZE = 0x200
BST_WAIT_TIME = 3369 	#BST code wait time
FP_WAIT_TIME = 300			#Fingerprint code wait time
ID_WAIT_TIME = 300			#Image decay wait time	
MEMORY_START = "3100"
DATA_START = "3610"

CHIP_NUMBER = str(9)
delayHigh = '11'
delayLow =  'B3'
acc = 99
logging.debug('Delay is '+delayHigh+' '+delayLow)
logging.debug("Embedding target time into code")
os.system('cp '+FINGERPRINT_CODE+'.txt '+FINGERPRINT_CODE+'_bst.txt')
r=open(FINGERPRINT_CODE+'_bst.txt','r+')
r.seek(-3,2)
r.write('@'+DATA_START+'\r\n'+delayLow+' '+delayHigh+'\r\nq\r\n')
r.close()

for i in range(20):
	logging.debug("Erasing the memory")
	os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -e ERASE_ALL')
	logging.debug("Writing fingerprinting code to flash")
	os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -w '+FINGERPRINT_CODE+'_bst.txt -v -z [RESET,VCC]')

	logging.debug("Write to flash complete, fingerprinting in progress")

	logging.debug("Waiting for the program to finish execution")
	time.sleep(FP_WAIT_TIME)

	logging.debug("Dumping main memory")
	os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -r [fingerprint.txt,MAIN]')

	logging.debug("Extracting data from main memory dump")
	f=file("fingerprint.txt").read()
	words=f.split()

	r=open(CHIP_NUMBER+'_'+str(i)+'_'+FINGERPRINT_OUTPUT+str(acc)+'.txt','w')
	r.write("@"+str(DATA_START)+"\r\n")

	position = int(MEMORY_START,16)
	linePos = 0
	for word in words:
		if position == int(DATA_START,16)+(256*256/2)+2:
			r.write("\r\n"+'q'+"\r\n")
			break
		if (position <= int(DATA_START,16)):
			position = position + 1
			continue
		r.write(word)
		position+=1
		linePos=linePos+1
		if(linePos==16):
			linePos=0
			r.write("\r\n")
		else:
			r.write(" ")

	r.close()
	os.system('rm fingerprint.txt')

os.system('rm '+FINGERPRINT_CODE+'_bst.txt')

quit()
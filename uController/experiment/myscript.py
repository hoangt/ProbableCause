import os
import sys
import time
import logging
import shutil

logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)


CHIP_NUMBER = str(2)

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

BST={99}#99,95,90}
TEMPERATURE = {40,50,60}
logging.debug("Starting execution")
logging.debug("Waiting for the chip to heat up")
time.sleep(60*60)
logging.debug("Starting with the experiments")
for temper in TEMPERATURE:
	TIMER_START = time.time()
	for acc in BST:
		logging.debug("Erasing the memory")
		os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -e ERASE_ALL')
		logging.debug("Writing BST code to flash")
		os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -w '+BST_CODE+str(acc)+'.txt -v -z [RESET,VCC]')
		logging.debug("Write to flash complete, BST in progress")

		logging.debug("Waiting for the program to finish execution")
		time.sleep(BST_WAIT_TIME)

		logging.debug("Dumping main memory")
		os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -r [bst.txt,MAIN]')
		os.system('cp bst.txt '+CHIP_NUMBER+'_'+str(temper)+'_'+'bst'+str(acc)+'.txt')
		logging.debug("Extracting target time from memory dump")
		f=file("bst.txt").read()
		words=f.split()
		last_delay=(int(DATA_START,16)-int(MEMORY_START,16)+1)
		while (words[last_delay+6]!='FF'):
			last_delay+=6
		delayHigh = words[last_delay+1]
		delayLow = words[last_delay]
		os.system('rm  bst.txt')

		logging.debug('Delay is '+delayHigh+' '+delayLow)

		logging.debug("Embedding target time to fingerprinting code")
		os.system('cp '+FINGERPRINT_CODE+'.txt '+FINGERPRINT_CODE+'_bst.txt')
		r=open(FINGERPRINT_CODE+'_bst.txt','r+')
		r.seek(-3,2)
		r.write('@'+DATA_START+'\r\n'+delayLow+' '+delayHigh+'\r\nq\r\n')
		r.close()
		 


		logging.debug("Writing fingerprinting code to flash")
		os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -w '+FINGERPRINT_CODE+'_bst.txt -v -z [RESET,VCC]')
		os.system('rm '+FINGERPRINT_CODE+'_bst.txt')
		logging.debug("Write to flash complete, fingerprinting in progress")

		logging.debug("Waiting for the program to finish execution")
		time.sleep(FP_WAIT_TIME)

		logging.debug("Dumping main memory")
		os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -r [fingerprint.txt,MAIN]')




		logging.debug("Extracting data from main memory dump")
		f=file("fingerprint.txt").read()
		words=f.split()

		r=open(CHIP_NUMBER+'_'+str(temper)+'_'+FINGERPRINT_OUTPUT+str(acc)+'.txt','w')
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


		logging.debug("Embedding image and decaytime into image decay code")
		os.system('cp '+IMGDECAY_CODE+'.txt '+IMGDECAY_CODE+'_img.txt')
		r=open(IMGDECAY_CODE+'_img.txt','r+')
		f=open('scale.txt','r')
		r.seek(-3,2)
		r.write('@'+DATA_START+'\r\n'+delayLow+' '+delayHigh+'\r\n')
		for line in f:
			r.write(line)
		r.close()
		f.close()


		logging.debug("Writing image decay code to flash")
		os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -w '+IMGDECAY_CODE+'_img.txt -v -z [RESET,VCC]')
		os.system('rm '+IMGDECAY_CODE+'_img.txt')
		logging.debug("Write to flash complete, image decay in progress")

		logging.debug("Waiting for the program to finish execution")
		time.sleep(ID_WAIT_TIME)

		logging.debug("Dumping main memory")
		os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -r [imagedecay.txt,MAIN]')


		logging.debug("Extracting data from main memory dump")
		f=file("imagedecay.txt").read()
		words=f.split()

		imgOutputLoc=int(DATA_START,16)+(0x10000/2)+2
		r=open(CHIP_NUMBER+'_'+str(temper)+'_'+IMAGEDECAY_OUTPUT+str(acc)+'.txt','w')
		r.write("@"+format(imgOutputLoc,'x')+"\r\n")
		position = int(MEMORY_START,16)
		linePos = 0


		for word in words:
			if position > imgOutputLoc+(30800): 
				r.write("\r\n"+'q'+"\r\n")
				break
			if (position <= imgOutputLoc):
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
		os.system('rm  imagedecay.txt')

	TIMER_END = time.time()
	logging.debug("Waiting to move to the next temperature")
	# Sleep for the remainder of 4 hours
	time.sleep(4*3600-(TIMER_END-TIMER_START))

quit()
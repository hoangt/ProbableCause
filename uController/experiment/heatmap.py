import os
import sys
import time
import logging
import shutil

logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)


CHIP_NUMBER = str(3)


HEATMAP_CODE = "dram_heatmap"
HEATMAP_OUTPUT='heatmap'
SEGMENT_SIZE = 0x200
HM_WAIT_TIME = 300			#Heatmap code wait time
MEMORY_START = "3100"
DATA_START = "3610"


TEMPERATURE = [35,45,55,65]
#INCREMENT = [35: 1, 45: 2, 55: 3, 65: 4]
# MIN_DELAY = INCREMNET
#MAX_DELAY = [35: 1, 45: 2, 55: 3, 65: 4]

WAIT_TIME = {35: 27000, 45: 12600, 55: 7200, 65: 5400}



logging.debug("Starting execution")
logging.debug("Starting with the experiments")
for temper in TEMPERATURE:
	logging.debug("Waiting for the chip to heat up")
	time.sleep(60*60)

	logging.debug("Erasing the memory")
	os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -e ERASE_ALL')

	logging.debug("Writing Heatmap code to flash")
	os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -w '+HEATMAP_CODE+str(temper)+'.txt -v -z [RESET,VCC]')
	logging.debug("Write to flash complete, fingerprinting in progress")

	logging.debug("Waiting for the program to finish execution ")
	logging.debug(WAIT_TIME[temper])

	time.sleep(WAIT_TIME[temper])

	logging.debug("Dumping main memory")
	os.system('/cygdrive/c/TI/MSP430Flasher_1.3.0/MSP430Flasher.exe -n MSP430F2618 -r [hm.txt,MAIN]')




	logging.debug("Extracting data from main memory dump")
	f=file("hm.txt").read()
	words=f.split()

	r=open(CHIP_NUMBER+'_'+str(temper)+'_'+HEATMAP_OUTPUT+'.txt','w')
	r.write("@"+str(DATA_START)+"\r\n")

	position = int(MEMORY_START,16)
	linePos = 0
	for word in words:
		if position == int(DATA_START,16)+(256*256)+2:
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
	os.system('rm hm.txt')



quit()
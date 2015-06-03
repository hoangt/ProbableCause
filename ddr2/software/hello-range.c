#include <stdio.h>
#include "spr_defs.h"

#define NUM_WORDS_TO_CHECK 0x10000                // A lot
#define CLOCK_FREQ ((unsigned long long)50000000) // 50 MHz
#define TIME_MIN (CLOCK_FREQ * 600)
#define TIME_MAX (CLOCK_FREQ * 2400)             
#define DECAY_LIMIT .99
#define MARGIN_OF_ERROR  .001
#define DRAM_COLS_PER_ROW 0X100
#define DRAM_BYTES_PER_COL 2
#define BYTES_PER_CHECK 4
#define ADDRESS_INCREMENT 1//(DRAM_COLS_PER_ROW*DRAM_BYTES_PER_COL/BYTES_PER_CHECK)

#define getSPR(spr)  ({         \
  unsigned int x;               \
  asm volatile(			        \
    "l.mfspr %0, r0, %1 \n\t"   \
    : "=r" (x)                  \
    : "i" (spr)                 \
  );		                    \
  x;                            \
})
#define getTicksLow() getSPR(SP_CLOCK_LOW);
#define getTicksHigh() getSPR(SP_CLOCK_HIGH);

#define refresh_off() asm volatile("l.nop 0xDEAD\n\t");
#define refresh_on() asm volatile("l.nop 0xBEEF\n\t");

void writeDataToDram(volatile unsigned int * pBaseAddress)
{
  unsigned int index;

  for(index = 0; index < (NUM_WORDS_TO_CHECK*ADDRESS_INCREMENT); index += ADDRESS_INCREMENT)
  {
    pBaseAddress[index] = 0xDEADBEEF;
  }
}

double calculateDecayPercentile(volatile unsigned int * pBaseAddress, const unsigned int pExpectedValue)
{
  unsigned int index;
  unsigned int failCount=0;
  for(index = 0; index < (NUM_WORDS_TO_CHECK*ADDRESS_INCREMENT); index += ADDRESS_INCREMENT)
  {
    // DRAM is x16 so check at that level
    if((pBaseAddress[index] & 0xFFFF) != (pExpectedValue & 0xFFFF))
      failCount++;
    if((pBaseAddress[index] & 0xFFFF0000) != (pExpectedValue & 0xFFFF0000))
      failCount++;
  }
  printf("Fail Count: %d\n\r",failCount);
  double decay = ((double) failCount)/(NUM_WORDS_TO_CHECK*ADDRESS_INCREMENT*2);
  printf("Decay percentile %e\n\r", decay);
  return decay;
}

int checkDramData(volatile unsigned int * pBaseAddress, const unsigned int pExpectedValue)
{
  unsigned int index;

  for(index = 0; index < (NUM_WORDS_TO_CHECK*ADDRESS_INCREMENT); index += ADDRESS_INCREMENT)
  {
    // DRAM is x16 so check at that level
    if((pBaseAddress[index] & 0xFFFF) != (pExpectedValue & 0xFFFF))
      printf("0x%8.8X 0\n\r", (unsigned int)&pBaseAddress[index]);
    if((pBaseAddress[index] & 0xFFFF0000) != (pExpectedValue & 0xFFFF0000))
      printf("0x%8.8X 1\n\r", (unsigned int)&pBaseAddress[index]);
  }

  return 0;
}

void printTime(const unsigned long long pTicks)
{
  unsigned int seconds = (unsigned int)(pTicks/CLOCK_FREQ);
  unsigned int minutes = seconds/60;
  seconds = seconds % 60;
  float fract = ((float)(pTicks % CLOCK_FREQ)) / CLOCK_FREQ;

  printf("%d:%d.%3.3d\n\r", minutes, seconds, (int)(fract*1000));
}

unsigned long long currentTime()
{
  unsigned long long ticks = (unsigned long long)getTicksHigh();
  ticks <<= 32;
  ticks += getTicksLow();

  return ticks;
}

int main(void)
{
  volatile unsigned int * farAddress = (unsigned int *)0x00040000;

  printf("Setting the value at address 0x%8.8X\n\r", (unsigned int)farAddress);
  printf("to address 0x%8.8X\n\r", (unsigned int)&farAddress[NUM_WORDS_TO_CHECK*ADDRESS_INCREMENT]);
  writeDataToDram(farAddress);

  printf("Disabling refresh\n\r");
  refresh_off();

  unsigned long long currentMinDelay = TIME_MIN;
  unsigned long long currentMaxDelay = TIME_MAX;
  unsigned long long timeToWait = (currentMinDelay+currentMaxDelay)/2;
  unsigned long long timeOfNextCheck = currentTime() + timeToWait;
  double decay = 0;
  while(1)
  {
    if(currentTime() > timeOfNextCheck)
    {
      // Turn refresh on, otherwise the DRAM locks the system
      refresh_on();

      printTime(timeToWait);
      decay = calculateDecayPercentile(farAddress, 0xDEADBEEF);
      if (((decay > DECAY_LIMIT) ? (decay - DECAY_LIMIT) : (DECAY_LIMIT - decay)) < MARGIN_OF_ERROR)
      {
		printf("Done1\n\r");
        return 0;
      }
      else
      {
        writeDataToDram(farAddress);
        // Setup the next refresh off period
        if(decay < DECAY_LIMIT)
        {
            currentMinDelay = timeToWait;
	    	printf("Min time moved\n\r");
        }
        else
        {
            currentMaxDelay = timeToWait;
	    	printf("Max time moved\n\r");
        }

        if(currentMaxDelay - currentMinDelay < 3)
		{
	    	printf("Done2\n\r");
        	return 0;
		}

        timeToWait = (currentMinDelay+currentMaxDelay)/2;
        timeOfNextCheck = currentTime() + timeToWait;
        refresh_off();
      }
    }
  }
  printf("Done\n\r");
  return 0;
}

#include <stdio.h>
#include "spr_defs.h"

#define STOP_PERCENT 		.999
#define NUM_WORDS_TO_CHECK 	0x10000  // A lot
#define CLOCK_FREQ 		((unsigned long long)50000000) // 50 MHz
#define TIME_INCREMENT 		(CLOCK_FREQ * 5)              // 5 seconds
#define START_TIME		(CLOCK_FREQ * 5)                 // seconds
//#define TIME_LIMIT 		(CLOCK_FREQ * 1500)            // 1500 seconds

#define DRAM_COLS_PER_ROW 512
#define DRAM_BYTES_PER_COL 2
#define BYTES_PER_CHECK 4
#define ADDRESS_INCREMENT 1

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
  unsigned int colCounter = 0;
  unsigned int rowCounter = 0;
  unsigned int data = 0xFFFFFFFF;

  for(index = 0; index < (NUM_WORDS_TO_CHECK*ADDRESS_INCREMENT); index += ADDRESS_INCREMENT)
  {
    pBaseAddress[index] = (data << 16) | data;

    if(((index >> 2) % 2) == 0)
    {
      ++colCounter;
      if(colCounter == (DRAM_COLS_PER_ROW / (BYTES_PER_CHECK / DRAM_BYTES_PER_COL)))
      {
        colCounter = 0;
        ++rowCounter;
        data = (rowCounter / 2) % 2 == 0 ? 0xFFFFFFFF : 0x0;
      }
    }
  }
}

int checkDramData(volatile unsigned int * pBaseAddress)
{
  unsigned int index;
  unsigned int flipCount = 0;
  unsigned int colCounter = 0;
  unsigned int rowCounter = 0;
  unsigned int data = 0xFFFFFFFF;

  for(index = 0; index < (NUM_WORDS_TO_CHECK*ADDRESS_INCREMENT); index += ADDRESS_INCREMENT)
  {
    if(((index >> 2) % 2) == 0)
    {
      // DRAM is x16 so check at that level
      if((pBaseAddress[index] & 0xFFFF) != (data & 0xFFFF))
      {
        printf("0x%8.8X 0\n\r", (unsigned int)&pBaseAddress[index]);
        ++flipCount;
      }    

      if((pBaseAddress[index] >> 16) != (data & 0xFFFF))
      {
        printf("0x%8.8X 1\n\r", (unsigned int)&pBaseAddress[index]);
        ++flipCount;
      }

      ++colCounter;
      if(colCounter == (DRAM_COLS_PER_ROW / (BYTES_PER_CHECK / DRAM_BYTES_PER_COL)))
      {
        colCounter = 0;
        ++rowCounter;
        data = (rowCounter / 2) % 2 == 0 ? 0xFFFFFFFF : 0x0;
      }
    }
  }

  return flipCount;
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
  printf("Stopping at %0.2f bits failed\n\r", STOP_PERCENT);

  writeDataToDram(farAddress);

  printf("Disabling refresh\n\r");
  refresh_off();

  unsigned long long timeToWait = START_TIME;
  unsigned long long timeOfNextCheck = currentTime() + timeToWait;
  while(1)
  {
    if(currentTime() > timeOfNextCheck)
    {
      // Turn refresh on, otherwise the DRAM locks the system
      refresh_on();

      printTime(timeToWait);
      unsigned int flipCount = checkDramData(farAddress);
      printf("Flips: %d\n\r", flipCount);

      timeToWait += TIME_INCREMENT;


      if(flipCount > ((STOP_PERCENT * (NUM_WORDS_TO_CHECK >> 1)) * 2))
      {
        printf("DONE\n\r");
	return 0;
      }

      /*/ Check for stop time
      if(timeToWait > TIME_LIMIT)
      {
        printf("DONE\n\r");
	return 0;
      }*/

      // Setup the next refresh off period
      timeOfNextCheck = currentTime() + timeToWait;
      refresh_off();
    }
  }

  return 0;
}

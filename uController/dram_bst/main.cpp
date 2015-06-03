// This program outputs a integer value representing the delay at which each word of DRAM fails
// The results are stored in Flash, starting at address SEGMENT_DATA_START

#include "io430.h"
#include "stdint.h"
#include "flash.h"
#include "flags.h"
#include "relay.h"
#include "dram.h"
#include "stdbool.h"

//#define DEBUG                   1
#define BLOCK_TIME_OFFSET       700   // clock ticks
#define READ_BLOCK_TIME_OFFSET  200   // clock ticks
#define MIN_DELAY               0   // .01 seconds
#define MAX_DELAY               60000  // .01 seconds
#define TICKS_PER_SECOND        10000

// Target accuracy and the acceptable margin of error in .01% increments
#define GOAL_ACCURACY		99
#define MAX_ACCURACY		100
#define MARGIN_OF_ERROR	        26    //represents .01% of total DRAM bits
#define TARGET_ACCURACY         (( ((unsigned long)DRAM_NUM_COLS) * DRAM_NUM_ROWS * DRAM_BITS_PER_WORD) * GOAL_ACCURACY/MAX_ACCURACY) 

// DRAM needs a garbage write to work correctly after being powered-up
#define dram_power_on() set_voltage(5); dram_init_chip()//power_on(); dram_init_chip()
#define dram_power_off() set_voltage(0)//power_off()

void writeDelayFlipsToFlash(const unsigned long pAddress, const unsigned int pDelay, const unsigned long pFlips)
{
  flash_wb(pAddress, pDelay);
  flash_wb(pAddress+2, pFlips >> 16);
  flash_wb(pAddress+4, pFlips);
}

 int main(void)
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  // Set clock to 1 MHz
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  
  // Set Flash clock
  FCTL2 = FWKEY + FSSEL0 + FN1;
  
  // Timer A set to count up based on system clock
  TACTL = MC1 | TASSEL1; 
  
  dac_init();
  flags_init();
  relay_init();
  dram_init_mcu();
  dram_power_on();

  unsigned long address =  0x3A00;//SEGMENT_DATA_START; //memory location for data storage
  
  unsigned int currentMinDelay = MIN_DELAY;
  unsigned int currentMaxDelay = MAX_DELAY;

  while(1)
  {
    // Reset dram row and col addresses
    int readRow = 0;
    int readCol = 0;
    int writeRow = 0;
    int writeCol = 0;
    unsigned long blockCounter = 0;
    unsigned long flipCounter = 0;
    unsigned int delay = (currentMinDelay / 2) + (currentMaxDelay / 2);
    unsigned long refreshDelayInBlocks = ((unsigned long)delay * TICKS_PER_SECOND) / (unsigned long)BLOCK_TIME_OFFSET + DRAM_NUM_COLS;
    
    TAR = 0;
    while(readRow < DRAM_NUM_ROWS)
    {
      // Debug check for too small of block offset
      #ifdef DEBUG
        if(TAR > BLOCK_TIME_OFFSET + 10)
          TAR = 0;
      #endif
      
      // Main unit of computation
      // Only start computation at periodic intervals  
      if(TAR > BLOCK_TIME_OFFSET)
      {
          // Reset timer
          TAR = 0;

          // Update the global sense of time
          blockCounter++;
        
          // Writing values into DRAM
	  // Avoid writing when all of DRAM written
          if(writeRow < DRAM_NUM_ROWS)
          {
              // Write the high value to cell
              dram_wb(writeRow, writeCol, (writeRow & 0x1) ? 0x0 : 0xF);

              // Update row and column for writes
              writeCol++;
              if(writeCol == DRAM_NUM_COLS)
              {
                  writeCol=0;
                  writeRow++;
              }
        }
        
        // Debug check for too small of block offset
        #ifdef DEBUG
          if(TAR > READ_BLOCK_TIME_OFFSET)
            TAR = 0;
        #endif
    
        // Busy wait until the time is right to do the read, test, and write
        while(TAR < READ_BLOCK_TIME_OFFSET)
          ;
	      // Only perform read and checks after the appropriate delay (in terms of blocks)
        if(blockCounter >= refreshDelayInBlocks)
        {
            unsigned char dataFromDRAM = dram_rb(readRow, readCol);
          
            // Count the number of unflipped bits in the word
            for (int s = 0; s < DRAM_BITS_PER_WORD; ++s)
                flipCounter += (readRow ^ (dataFromDRAM >> s)) & 0x1;

            // Update row and column for reads
            readCol++;
            if(readCol == DRAM_NUM_COLS)
            {
              readCol = 0;
              readRow++;
            }
        }
      }
    }
    
//    #ifdef DEBUG
      writeDelayFlipsToFlash(address, delay, flipCounter);
      address += 6;
 //   #endif
    
    // Stop searching for a delay when the number of bit flips is within range
    if(((flipCounter > TARGET_ACCURACY) ? (flipCounter - TARGET_ACCURACY) : (TARGET_ACCURACY - flipCounter)) < MARGIN_OF_ERROR)
    {
        writeDelayFlipsToFlash(address, delay, flipCounter);
        return 0;
    }
    // Binary search for the next delay
    else
    {
        if(flipCounter < TARGET_ACCURACY)
        {
            currentMaxDelay = delay;
        }
        else
        {
            currentMinDelay = delay;
        } 
    }

    // Stop the system from oscillating around a value not within the margin
    if(currentMinDelay+3 > currentMaxDelay)
    {
      writeDelayFlipsToFlash(address, delay, flipCounter);
      return 1;
    }
  }
}

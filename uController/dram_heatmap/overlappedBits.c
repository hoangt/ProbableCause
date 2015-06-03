// This program outputs an 8-bit value representing the delay at which each bit of DRAM fails
// To save time, this version interleaves the writing and the reading/testing phases
// The results are stored in Flash, starting at address SEGMENT_DATA_START
// Supports up to 255*DELAY_INCREMENT seconds of delay
// Flash may not hold results for all of DRAM

#include "io430.h"
#include "stdint.h"
#include "flash.h"
#include "flags.h"
#include "relay.h"
#include "dram.h"

//#define DEBUG                   1
#define BLOCK_TIME_OFFSET       850
#define READ_BLOCK_TIME_OFFSET  150
#define MIN_DELAY               2
// Requires that MAX_DELAY/2 < 256 to fit in 8-bit result
#define MAX_DELAY               450
#define DELAY_INCREMENT         2
#define TICKS_PER_SEC           1000000

// DRAM needs a garbage write to work correctly after being powered-up
#define dram_power_on() power_on(); dram_init_chip()
#define dram_power_off() power_off()

int main(void)
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  // Set clock to 1 MHz
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  
  // Set Flash clock
  FCTL2 = FWKEY + FSSEL0 + FN1;
  
  // Setup the GPIO pins
  flags_init();
  relay_init();
  dram_init_mcu();
  
  // Power-on DRAM and perform garbage writes
  dram_power_on();
  
  // Timer A set to count up based on system clock
  TACTL = MC1 | TASSEL1; 

  for(int delay = MIN_DELAY; delay <= MAX_DELAY; delay = delay + DELAY_INCREMENT){
    // Frame the delay in terms of blocks executed
    // Add the number of columns to result to account for refresh of entire row
    // when writing any word in the row
    unsigned long blockToStartTesting = (delay*TICKS_PER_SEC)/BLOCK_TIME_OFFSET + DRAM_NUM_COLS;
    
    // Reset dram row and col addresses
    int readRow = 0;
    int readCol = 0;
    int writeRow = 0;
    int writeCol = 0;
    unsigned long blockCounter = 0;
    unsigned long baseAddressFlash = SEGMENT_DATA_START;
    TAR = 0;
      
    while(readRow < DRAM_NUM_ROWS){
      // Debug check for too small of block offset
      #ifdef DEBUG
        if(TAR > (BLOCK_TIME_OFFSET + 10))
          TAR = 0;
      #endif
      
      // Main unit of computation
      // Only start computation at periodic intervals
      if(TAR > BLOCK_TIME_OFFSET){
        // Reset timer
        TAR = 0;
      
	// Update the global sense of time
	++blockCounter;

        // Avoid doing writes after all of DRAM written once
        if(writeRow < DRAM_NUM_ROWS){
          dram_wb(writeRow, writeCol, (writeRow & 0x1) ? 0x0 : 0xF);
	  
	  // Update row and column for writes
	  ++writeCol;
	  if(writeCol == DRAM_NUM_COLS){
	    ++writeRow;
	    writeCol = 0;
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
      
        // Skip testing until past delay
        if(blockCounter >= blockToStartTesting){
          // Read the value that remains in DRAM after the delay
          char dataFromDRAM = dram_rb(readRow, readCol);

          // Make sure that Flash has enough space to store the results
          if(baseAddressFlash < SEGMENT_DATA_END)
          {
            // Read the four bytes needed from Flash in aggregate
            unsigned long currResults = flash_read_long(baseAddressFlash);
            // Init to Flash startup state: no results written yet
            unsigned long newResults = 0xFFFFFFFF;
            unsigned long currInc = 0xFFFFFF00 | delay/DELAY_INCREMENT;
          
            // If this is the first failure (something other than 0xF in Flash)
            // and the data read from DRAM doesn't match what we wrote,
            // write the delay to Flash
            // Check for a failure at each bit of the word
            // Store the failing delay if this is the first failure for this bit
            if(((dataFromDRAM ^ readRow) & 0x1) == 0 && (currResults & 0xFF)  == 0xFF){
                newResults = newResults & currInc;
            }
            if((((dataFromDRAM >> 1) ^ readRow) & 0x1) == 0 && (currResults & 0xFF00)  == 0xFF00){
                newResults = newResults & ((currInc << 8) | 0xFF);
            }
            if((((dataFromDRAM >> 2) ^ readRow) & 0x1) == 0 && (currResults & 0xFF0000)  == 0xFF0000){
                newResults = newResults & ((currInc << 16) | 0xFFFF);
            }
            if((((dataFromDRAM >> 3) ^ readRow) & 0x1) == 0 && (currResults & 0xFF000000)  == 0xFF000000){
                newResults = newResults & ((currInc << 24) | 0xFFFFFF);
            }
          
            // Write results in aggregate to Flash
            // Note that Flash only goes from 1 to 0, so writing extra 1's will not change a 0
            flash_write_long(baseAddressFlash, newResults);
          
            // Update the Flash address that we are working with next cycle
            // Each word of DRAM requires 8-bits of Flash per bit for the result
            baseAddressFlash += 4;
          }
          
          // Update row and column for reads
          ++readCol;
          if(readCol == DRAM_NUM_COLS){
            ++readRow;
            readCol = 0;
          }
        }
      }
    }
  }
  
  return 0;
}

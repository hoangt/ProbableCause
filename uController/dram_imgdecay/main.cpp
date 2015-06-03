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
#define BLOCK_TIME_OFFSET       450
#define READ_BLOCK_TIME_OFFSET  150
#define TICKS_PER_SEC           10000
#define SIDE                    0


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

  unsigned int delay = flash_rb(SEGMENT_DATA_START); //delay in 0.01 intervals read from bst

  // Frame the delay in terms of blocks executed
  // Add the number of columns to result to account for refresh of entire row
  // when writing any word in the row
  unsigned long blockToStartTesting = ((uint32_t)delay*TICKS_PER_SEC)/BLOCK_TIME_OFFSET + DRAM_NUM_COLS;
  
  //for (uint32_t i = 0x3200; i<0x4200;
  
  // Reset dram row and col addresses
  int readRow =  0;
  int readCol =  0;
  int writeRow = 0;
  int writeCol = 0;
  unsigned long blockCounter = 0;
  unsigned long flashAddressRead = SEGMENT_DATA_START + 2; // Add 2 to account for delay being passed in Flash
  // Start writing to Flash the size of DRAM from where you start reading
  unsigned long flashAddressWrite = flashAddressRead + (unsigned long)DRAM_NUM_COLS*(DRAM_NUM_ROWS/2);
  unsigned char img;
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
      if(writeRow < (DRAM_NUM_ROWS)){
	// DRAM writes are 4 bits, but Flash reads are 8 bits
	// Every other DRAM write, get a new byte from Flash
        if((writeCol & 0x1) == 0){
          img = flash_read_byte(flashAddressRead);
	  dram_wb(writeRow, writeCol, (img >> 4) & 0xF);
          ++flashAddressRead;
        }
        else {
	  dram_wb(writeRow, writeCol, img & 0xF);
	}

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
        char dataFromDRAM = dram_rb(readRow, readCol); //First half byte

      	// Even cols in MSBs to match bit ordering
      	char dataToWrite = (((readCol & 0x1) == 0) ? (dataFromDRAM << 4) | 0xF : dataFromDRAM | 0xF0);

      	// Write results in aggregate to Flash
      	// Because write to flash can only change 1->0 this operation should not write over even coloumn results.
      	flash_write_byte(flashAddressWrite, dataToWrite);
                
      	// Update the Flash address that we are working with next cycle.
	// This only happens every other time because results from 2 dram reads are aggeregated.
	flashAddressWrite += ((readCol & 0x1) == 1) ? 1 : 0;

        // Update row and column for reads
        ++readCol;
        if(readCol == DRAM_NUM_COLS){
          ++readRow;
          readCol = 0;
        }
      }
    }
  }
  return 0;
}

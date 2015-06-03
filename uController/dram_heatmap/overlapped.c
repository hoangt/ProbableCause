// This program outputs a integer value representing the delay at which each word of DRAM fails
// To save time, this version interleaves the writing and the reading/testing phases
// The results are stored in Flash, starting at address SEGMENT_DATA_START

#include "io430.h"
#include "stdint.h"
#include "flash.h"
#include "flags.h"
#include "relay.h"
#include "dram.h"

#define BLOCK_TIME_OFFSET       775
#define READ_BLOCK_TIME_OFFSET  290
#define MIN_DELAY               1   // DELAY_INCREMENT
#define MAX_DELAY               50 // 50, 75, 175, 275
#define DELAY_INCREMENT         1   // 1,  1,  2, 2
//#define EXPERIMENT_TIME (MAX_DELAY/DELAY_INCREMENT)*(MAX_DELAY/2+(BLOCK_TIME_OFFSET*DRAM_NUM_ROWS*DRAM_NUM_COLS/1000000+1))

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
    // Frame the delay in terms of rows written
    // Add 1 to result to account for refresh of entire row
    // when writing any word in the row
    long writeRowToStartTesting = (delay*1000000)/BLOCK_TIME_OFFSET/DRAM_NUM_COLS + 1;
    
    // Reset dram row and col addresses
    int readRow = 0;
    int readCol = 0;
    int writeRow = 0;
    int writeCol = 0;
    TAR = 0;
    unsigned long flashAddress = SEGMENT_DATA_START;  
    while(readRow < DRAM_NUM_ROWS){
      

      if(TAR > BLOCK_TIME_OFFSET){
        // Reset timer
        TAR = 0;
      
        // Avoid doing writes after all of DRAM written once
        if(writeRow < DRAM_NUM_ROWS){
          dram_wb(writeRow, writeCol, ((writeRow % 2) == 0) ? 0xF : 0x0);
        }
      
        // Update row and column for writes
        // Outside of if block because writeRow acts as a global sense of time
        ++writeCol;
        if(writeCol == DRAM_NUM_COLS){
          ++writeRow;
          writeCol = 0;
        }
        
        // Busy wait until the time is right to do the read, test, and write
        while(TAR < READ_BLOCK_TIME_OFFSET)
          ;
      
        // Skip testing until past delay
        if(writeRow >= writeRowToStartTesting){
          char dataFromDRAM = dram_rb(readRow, readCol);

          // If this is the first failure (something other than 0xFF in Flash)
          // and the data read from DRAM doesn't match what we wrote,
          // write the delay to Flash
          char dataToCheckFor = ((readRow % 2) == 0) ? 0xF : 0x0; // Accomodate DRAM wordline bias
          if(dataFromDRAM != dataToCheckFor && flash_read_byte(flashAddress) == 0xFF){
            flash_write_byte(flashAddress, delay/DELAY_INCREMENT);
          }
      
          // Update row and column for reads
          ++readCol;
          if(readCol == DRAM_NUM_COLS){
            ++readRow;
            readCol = 0;
          }

	  // Each DRAM word requires one flash byte of storage
	  ++flashAddress;
        }
      }
    }
  }
  
  return 0;
}

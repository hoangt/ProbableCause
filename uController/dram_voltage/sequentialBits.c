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
#define TEST_VOLTAGE            1
#define MAX_DELAY               15000  // .01 ige seconds
#define STEP_DELAY              1000  // .01 seconds
#define MIN_DELAY               STEP_DELAY // .01 seconds
#define TICKS_PER_SECOND        10000
#define FREQ                    1000000

// DRAM needs a garbage write to work correctly after being powered-up
#define dram_power_on() set_voltage(5); dram_init_chip();
#define dram_power_off() set_voltage(0)

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

  for(unsigned int delay = MIN_DELAY; delay <= MAX_DELAY; delay += STEP_DELAY)
  {
    dram_power_on();
    // Reset dram row and col addresses
    int readRow = 0;
    int readCol = 0;
    int writeRow = 0;
    int writeCol = 0;

    // Writing values into DRAM
    // Avoid writing when all of DRAM written
    while(writeRow < DRAM_NUM_ROWS)
    {
      // Write the high value to cell
      // Use hidden refresh to keep all of DRAM refreshed
      dram_wb_hr(writeRow, writeCol, (writeRow & 0x1) ? 0x0 : 0xF);

      // Update row and column for writes
      writeCol++;
      if(writeCol == DRAM_NUM_COLS)
      {
        writeCol=0;
        writeRow++;
      }
    }

    // Delay initiated
    set_voltage(TEST_VOLTAGE);

    for(unsigned int d = 0; d < delay; d++)
      __delay_cycles(TICKS_PER_SECOND);

    //dram_power_on();

    unsigned long baseAddressFlash =  0x3A00; //memory location for data storage
    unsigned int validWords = 0xFFFF;

    // Reading values into DRAM
    // Avoid reading when all of DRAM is read
    while(readRow < DRAM_NUM_ROWS)
    {
      // Make sure that Flash has enough space to store the results
      if(baseAddressFlash < SEGMENT_DATA_END)
      {
        // Read the value that remains in DRAM after the delay
        char dataFromDRAM = dram_rb(readRow, readCol);

        // If this is the first failure (something other than 0xFF in Flash)
        // and the data read from DRAM doesn't match what we wrote,
        // write the delay to Flash
        char dataToCheckFor = ((readRow % 2) == 0) ? 0xF : 0x0; // Accomodate DRAM wordline bias
        if(dataFromDRAM != dataToCheckFor)
        {
          --validWords;

          if(flash_read_byte(baseAddressFlash) == 0xFF)
            flash_write_byte(baseAddressFlash, delay/(FREQ/TICKS_PER_SECOND));
        }

        ++baseAddressFlash;
      }

      // Update row and column for reads
      readCol++;
      if(readCol == DRAM_NUM_COLS)
      {
        readCol = 0;
        readRow++;
      }
    }

    // Quit when more than 10% words decayed
    if(validWords < (.9 * 0xFFFF))
      return 0;
  }
}

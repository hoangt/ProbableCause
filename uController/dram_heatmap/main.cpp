// This program outputs a integer value representing the delay at which each word of DRAM fails
// To convert the delay to time: delay * DELAY_STEP (in cycles)
// The results are stored in Flash, starting at address SEGMENT_DATA_START

#include "io430.h"
#include "stdint.h"
#include "flash.h"
#include "flags.h"
#include "relay.h"
#include "dram.h"

// Start and limits (in cycles) for the refresh/power delay
#define DELAY_STEP           50000
#define DELAY_START       (5000000/DELAY_STEP) //  100
#define DELAY_INCREMENT   (50000000/DELAY_STEP) //  100
#define DELAY_LIMIT     (300000000/DELAY_STEP) // 6000

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
  
  flags_init();
  relay_init();
  dram_init_mcu();
  dram_power_on();
  
  int* address = (int*)SEGMENT_DATA_START; //memory location for data storage

  // Span DRAM cells
  for(char i = 20; i != 21; ++i){
    for(char j = 20; j != 21; ++j){
      unsigned char dataFromDRAM;
      
      // Run through delays, until a cell fails
      for(int delay = DELAY_START; delay <= DELAY_LIMIT; delay += DELAY_INCREMENT){
	// Write all 1's
        dram_wb(i, j, 0xF);

	dram_power_off();
	// Builtin function __delay_cycles, broken down into less than UINT_MAX chunks
	for(int delaySteps = 0; delaySteps < delay; ++delaySteps){
	  __delay_cycles(DELAY_STEP);
	}
	dram_power_on();
        
        dataFromDRAM = dram_rb(i, j);
	if(dataFromDRAM != 0xF){
          flash_wb(address, delay);
          break;
	}
	
	// Write all 0's
        dram_wb(i, j, 0x0);
	
	dram_power_off();
	// Builtin function __delay_cycles, broken down into less than UINT_MAX chunks
	for(int delaySteps = 0; delaySteps < delay; ++delaySteps){
	  __delay_cycles(DELAY_STEP);
	}
	dram_power_on();

        dataFromDRAM = dram_rb(i, j);
	if(dataFromDRAM != 0x0){
          flash_wb(address, delay);
          break;
	}
      }
      
      address++;
    }
  }

  return 0;
}

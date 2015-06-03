
#include "io430.h"
#include "stdint.h"
#include "flash.h"
#include "flags.h"
#include "relay.h"
#include "dram.h"



int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  //uint32_t value = 0x3F804;
  //int* testaddress = (int*) 0x3300;
  //char* address2 = (char*) 0x3300;
  //write_Seg(address2,0x12);
  //flash_wb(testaddress,value>>16&0xFFFF);
  //flash_wb(testaddress+1,value&0xFFFF);
  //address++;
  // write_Seg(address2,0x34);
  //flash_wb(address,0x5678);
  //address++;
  //flash_wb(address,0x9ABC);


  // Set ports direction
  P1SEL = 0x00;
  P2SEL = 0x00;
  P3SEL = 0x00;
  P1DIR = 0xFF; //Control bits
  P4DIR = 0xFF; //Address bits
  P1OUT = 0x00;
  P2OUT = 0x00;
  set_relay_high();
  set_ctr1_low();
  set_ctr2_low();
  
  char i=0x00;
  char j=0x00;
  unsigned char res;
  int* address = (int*)SEGMENT_DATA_START; //memory location for data storage

  for(unsigned long cc=0;cc<TESTCOUNT;cc++){
    for(unsigned int dlim=0;dlim<TESTLIMIT+1;dlim+=TESTSTEP){
      set_ctr1_high();
      //write cycle
      do{
        do{
          dram_wb(i,j,0xF);
          j++;
        }while(j!=0x00);
        i++;
      }while(i!=0x00);
      set_ctr1_low();
      
      //refresh cycle
      //do{
      //    dram_ref(i);
      //    i++;
      //}while(i!=0x00);
    
      set_ctr2_high();
      set_relay_low();
      for(int dcycles=0;dcycles<dlim;dcycles++)
        __delay_cycles(SECDELAY);
      set_relay_high();
      set_ctr2_low();
     //read cycle 
      uint32_t counter=262144; //0x100*0x100*4; //Assuming all bits should be 1, we will decrease the counter if we find a 0
      do{
        do{
          res=dram_rb(i,j);
          while(res!=0){
            if(res&1==1){
              counter--;
            }
              res>>=1;
          }
          j++;
        }while(j!=0x00);
        i++;
      }while(i!=0x00);
      
      
      flash_wb(address,counter>>16&0xFFFF);
      address++;
      flash_wb(address,counter&0xFFFF);
      address++;
    }
  }

    return 0;
}

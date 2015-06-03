// DRAM is 256x256x4
#define DRAM_NUM_COLS      256
#define DRAM_NUM_ROWS      256
#define DRAM_BITS_PER_WORD 4
// Signal port masks
#define W_MASK       0x2
#define OE_MASK      0x4
#define RAS_MASK     0x8
#define CAS_MASK     0x10
#define DATA_MASK    0xF
#define ADDRESS_MASK 0xFF

// DRAM interface signals
#define set_w_high()            P1OUT |= W_MASK
#define set_w_low()             P1OUT &= ~W_MASK
#define set_oe_high()           P1OUT |= OE_MASK
#define set_oe_low()            P1OUT &= ~OE_MASK
#define set_ras_high()          P1OUT |= RAS_MASK
#define set_ras_low()           P1OUT &= ~RAS_MASK
#define set_cas_high()          P1OUT |= CAS_MASK
#define set_cas_low()           P1OUT &= ~CAS_MASK
#define set_address(pos)        P4OUT = (pos) // P4 is 8 bits
#define read_data()             (P3IN & DATA_MASK) // Data is 4 bits
#define write_data(v)           P3OUT = (P3OUT & ~DATA_MASK) | ((v) & DATA_MASK) // P3 is 8 bits and data is 4
#define dram_init_chip()        dram_wb(0, 1, 0xF); dram_wb(1, 1, 0xF)

void dram_init_mcu(void){
  // Clear bits that need to be GPIOs
  P1SEL = P1SEL & ~(W_MASK | OE_MASK | RAS_MASK | CAS_MASK); // Control bits
  P3SEL = P3SEL & ~DATA_MASK; // Data bits
  P4SEL = P4SEL & ~ADDRESS_MASK; // Address bits
  // Set port direction: 1 = output
  P1DIR = P1DIR | (W_MASK | OE_MASK | RAS_MASK | CAS_MASK); // Control bits
  P3DIR = P3DIR & ~DATA_MASK; // Data bits
  P4DIR = P4DIR | ADDRESS_MASK; // Address bits
  // Set initial output values to 0
  P1OUT = P1OUT & ~(W_MASK | OE_MASK | RAS_MASK | CAS_MASK); // Control bits
  P4OUT = P4OUT & ~ADDRESS_MASK; // Address bits
}

void dram_ref(char row){
  set_ras_high();
  set_address(row);
  set_ras_low();
  set_ras_high();
  set_address(0x0);
}

void dram_wb(char row, char column, char value){
  P3DIR |= DATA_MASK; // Lower 4 bits out to DRAM data
  set_w_low();
  set_address(row);
  set_ras_low();
  set_address(column);
  write_data(value); 
  set_cas_low();
  set_cas_high();
  set_ras_high();
}

char dram_rb(char row, char column){
  P3DIR &= ~DATA_MASK; // Lower 4 bits in from DRAM data
  set_w_high();
  set_oe_low();
  set_address(row);
  set_ras_low();
  set_address(column);
  set_cas_low();
  char value = read_data();
  set_cas_high();
  set_ras_high();

  return value;
}

// write byte with hidden refresh---entire DRAM refreshed
// undocumented
void dram_wb_hr(char row, char column, char value){
  P3DIR |= DATA_MASK; // Lower 4 bits out to DRAM data
  set_w_low();
  set_address(row);
  set_ras_low();
  set_address(column);
  write_data(value); 
  set_cas_low();
  set_cas_high();
  set_ras_high();
  set_ras_low();
  set_ras_high();
}

// read byte with hidden refresh---entire DRAM refreshed
char dram_rb_hr(char row, char column){
  P3DIR &= ~DATA_MASK; // Lower 4 bits in from DRAM data
  set_w_high();
  set_oe_low();
  set_address(row);
  set_ras_low();
  set_address(column);
  set_cas_low();
  char value = read_data();
  set_cas_high();
  set_ras_high();
  set_ras_low();
  set_ras_high();

  return value;
}
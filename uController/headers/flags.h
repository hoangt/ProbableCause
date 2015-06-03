// Signal port masks
#define FLAG1_MASK   0x20
#define FLAG2_MASK   0x40

// Flags to aid processing of DAq output
#define set_flag1_high()        P1OUT |= FLAG1_MASK
#define set_flag1_low()         P1OUT &= ~FLAG1_MASK
#define set_flag2_high()        P1OUT |= FLAG2_MASK
#define set_flag2_low()         P1OUT &= ~FLAG2_MASK
#define toggle_flag1()          P1OUT ^= FLAG1_MASK
#define toggle_flag2()          P1OUT ^= FLAG2_MASK

void flags_init(void){
  // Clear bits that need to be GPIOs
  P1SEL = P1SEL & ~(FLAG1_MASK | FLAG2_MASK);
  // Set port direction: 1 = output
  P1DIR = P1DIR | (FLAG1_MASK | FLAG2_MASK);
  // Set initial output values to 0
  P1OUT = P1OUT & ~(FLAG1_MASK | FLAG2_MASK);
}

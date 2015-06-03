// Signal port masks
#define RELAY_MASK   0x1

// Power gating relay
#define power_on()              P1OUT |= RELAY_MASK
#define power_off()             P1OUT &= ~RELAY_MASK

void relay_init(void){
  // Clear bits that need to be GPIOs
  P1SEL = P1SEL & ~RELAY_MASK;
  // Set port direction: 1 = output
  P1DIR = P1DIR | RELAY_MASK;
  // Set initial output values to 1
  P1OUT = P1OUT | RELAY_MASK;
}

void dac_init(void){
	ADC12CTL0 = REF2_5V + REFON;              // Internal 2.5V ref on
	TACCR0 = 13600;                           // DELAY to allow Ref to settle
	TACCTL0 |= CCIE;                          // Compare-mode interrupt.
  TACTL = TACLR + MC_1 + TASSEL_2;          // up mode, SMCLK
 // __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, enable interrupts
  return;
}

void set_voltage(float v){
  v=v/2; // Op-amp effect
	if (v>2.5 || v<0)
		return;
  TACCTL0 &= ~CCIE;                         // Disable timer interrupt
  __disable_interrupt();                    // Disable Interrupts
  DAC12_0CTL = DAC12IR + DAC12AMP_5 + DAC12ENC; // Int ref gain 1
  float vin = v/2.5;
  vin=vin>1?1:vin;
  DAC12_0DAT = (unsigned int) (vin*0xFFF);   //(2.5V = 0x0FFFh)
  __bis_SR_register(GIE);       // Enter LPM0, enable interrupts
return;
}



// DELAY ISR
#pragma vector = TIMERA0_VECTOR
__interrupt void TA0_ISR(void)
{
  TACTL = 0;                                // Clear Timer_A control registers
  __bic_SR_register_on_exit(LPM0_bits);     // Exit LPMx, interrupts enabled
}

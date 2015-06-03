// Define the flash memory range
// Where we write results
#define SEGMENT_DATA_START     0x3A10
#define SEGMENT_DATA_END       0x20000
#define FLASH_SEGMENT_SIZE     0X200
// Make sure user data is aligned to a segment to ensure that we can erase user data without erasing program data
#define FLASH_DATA_START       (((SEGMENT_DATA_START % FLASH_SEGMENT_SIZE) == 0) ? SEGMENT_DATA_START : (SEGMENT_DATA_START-(SEGMENT_DATA_START % FLASH_SEGMENT_SIZE) + FLASH_SEGMENT_SIZE))
#define FLASH_INT_CAP          ((SEGMENT_DATA_END - SEGMENT_DATA_START)/2)

// Write a byte to Flash
void flash_write_byte(unsigned long address , char value){
  __disable_interrupt();
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  __data20_write_char(address, value);      // Write value to flash
  while((FCTL3 & BUSY));
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
  __enable_interrupt();
}

// Write 32-bits to Flash
void flash_write_long(unsigned long address , unsigned long value){
  __disable_interrupt();
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  __data20_write_long(address, value);      // Write value to flash
  while((FCTL3 & BUSY));
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
  __enable_interrupt();
}

void flash_wb(unsigned long address , int value){
  __disable_interrupt();
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  __data20_write_short(address, value);     // Write value to flash
  while((FCTL3 & BUSY));
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
  __enable_interrupt();
}

// Read a byte from Flash
char flash_read_byte(unsigned long address){
  char ret;
  __disable_interrupt();
  ret = __data20_read_char(address);
  __enable_interrupt();
  return ret;
}

// Read 32-bits from Flash
unsigned long flash_read_long(unsigned long address){
  unsigned long ret;
  __disable_interrupt();
  ret = __data20_read_long(address);
  __enable_interrupt();
  return ret;
}

int flash_rb(unsigned long address){
  int ret;
  __disable_interrupt();
  ret = __data20_read_short(address);
  __enable_interrupt();
  return ret;
}

void flash_erase(unsigned long address)     //flash segments are 0x200 wide.
{
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  __data20_write_short(address, 0);         // Dummy write to erase Flash seg
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}


void write_Seg(char *Flash_ptr, char value)
{
  unsigned int i;
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  *Flash_ptr = 0;                           // Dummy write to erase Flash seg

  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

  for (i = 0; i < 64; i++)
  {
    *Flash_ptr++ = value;                   // Write value to flash
  }

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

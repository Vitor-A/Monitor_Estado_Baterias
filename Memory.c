void write_config(unsigned int32 address, unsigned int8 *dataptr, unsigned int8 count) {
   
  address -= getenv("EEPROM_ADDRESS");
  for(int8 i = 0; i < count; i++) {
    if(read_eeprom(address) != dataptr[i]) {
       write_eeprom(address, dataptr[i]);
    }
    address++;
  }
   
}

void read_config(unsigned int32 address, unsigned int8 *dataptr, unsigned int8 count) {
   
  address -= getenv("EEPROM_ADDRESS");
  for(int8 i = 0; i < count; i++) {
    dataptr[i] = read_eeprom(address);
    address++;
  }
  
}
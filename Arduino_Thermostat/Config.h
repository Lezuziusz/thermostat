#include "debug.h"
#include <EEPROM.h>

struct ConfigSet {
  float temp;
  bool cooling;
  float eps;
};

#define EEPROM_ADR_CFG 1
#define EEPROM_ADR_CFGCRC(x) EEPROM_ADR_CFG+sizeof(x)

class ConfigEE {
  public:
    ConfigSet data;
  
    ConfigEE(){
      setDefaults();
    }

  void setDefaults(){
    data.temp = 24.00;
    data.cooling = true;
    data.eps = 0.5;
  }

  void print(){
    Serial.println("Configuration:");
    Serial.print("Cooling:");
    Serial.println(data.cooling);
    Serial.print("Temp:");
    Serial.println(data.temp);
    Serial.print("Epsilon:");
    Serial.println(data.eps);
  }

  // read configuration stored in EEPROM
  // return 1 if stored configuration's CRC is not matching stored CRC
  // return 0 if OK
  byte read(){
    unsigned long crcE;
    unsigned long crcM;
    
    // read stored values
    EEPROM.get(EEPROM_ADR_CFG, data);
    
    // read stored CRC
    EEPROM.get(EEPROM_ADR_CFGCRC(data), crcE);
    dp(F("Stored CRC = "));
    dpln2(crcE,HEX);
    
    // calculate CRC for stored settings
    crcM = mem_crc(&data, sizeof(data));
    dp(F("Config CRC = "));
    dpln2(crcM,HEX);  
    
    return (crcM == crcE);
    
  }
  
  // write configuration to EEPROM
  void write(){
    unsigned long crc;
    
    crc = mem_crc(&data, sizeof(data));
    EEPROM.put(EEPROM_ADR_CFG, data);
    EEPROM.put(EEPROM_ADR_CFGCRC(data), crc);
    dpln(F("Configuration written to EEPROM"));
  
    if (!read()){    
      dpln(F("Cfg write failed."));
      
      setDefaults();
    }
      
  }

private:
  unsigned long mem_crc(void* obj, unsigned int objSize) {
    byte* a;
    a = (byte*)obj;
  
    const unsigned long crc_table[16] = {
      0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
      0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
      0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
      0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };
  
    unsigned long crc = ~0L;
  
    for (unsigned int index = 0 ; index < objSize  ; ++index) {
      crc = crc_table[(crc ^ *(a+index)) & 0x0f] ^ (crc >> 4);
      crc = crc_table[(crc ^ (*(a+index) >> 4)) & 0x0f] ^ (crc >> 4);
      crc = ~crc;
    }
    return crc;
  }
};


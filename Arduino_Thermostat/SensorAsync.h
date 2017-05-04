#include "debug.h"
#include <OneWire.h>

#define READ_DELAY  1000  // at least 750ms are needed for proper temp reading

// inspired by http://playground.arduino.cc/Learning/OneWire

class TempSensor {
  private:
    int pin;
    OneWire *owb;             // pin connected to one-wire-bus
    float lastTemp;
    float currentTemp;
    byte addr[8];
    unsigned long nextReading;
    bool ready;

  float getTemp_prim(){
    byte data[2];
    int16_t reading;
    float result;

    owb->reset();
    owb->select(addr);    // use device found during initialization
    owb->write(0xBE);     // read scratch pad

    // read temperature
    for (byte i=0;i<2;i++)
      data[i] = owb->read();

    // convert 16bits to temperature
    result = (data[1]<<8) | data[0];
    result = result / 16;
    
    // and ask sensor to prepare data so we have them available for the next reading
    owb->reset();
    owb->select(addr);  // // use device found during initialization 
    owb->write(0x44,1); // request temperature conversion, leave parasite power ON at the end
              
    return result;
  }
    

  public:
    TempSensor(int pin){
      ready = false;
      this->pin = pin;
      owb = new OneWire(pin);

      if ( !owb->search(addr)) {
        dpln("No more addresses.\n");
        owb->reset_search();
        delay(250);
        return;
      }
      
      dp("Temperature sensor found: ");
      for(byte i = 0; i < 8; i++) {
        dp2(addr[i], HEX);
        dp(" ");
      }
      dpln();
    
      if ( OneWire::crc8( addr, 7) != addr[7]) {
        dp("Sensor CRC is not valid!\n");
        return;
      }
      
      if ( addr[0] != 0x28) {  //0x10 for DS18S20
        dp("Device is not a DS18B20 family device.\n");
        return;
      }
      
      lastTemp = getTemp_prim();
      currentTemp = lastTemp;
      nextReading = millis() + READ_DELAY;
      ready = true;
    }

  bool tempChanged(){
    return lastTemp != currentTemp;
  }

  float getTemp(){
    if (nextReading < millis()){
      lastTemp = currentTemp;
      currentTemp = getTemp_prim();
      nextReading = millis() + READ_DELAY;
    }
    return currentTemp;
  }

  bool isReady(){
    return ready;
  }
    

};

#include "debug.h"
#include <DallasTemperature.h>

class TempSensor {
  private:
    int pin;
    OneWire *owb;             // pin connected to one-wire-bus
    DallasTemperature *dst;   // Dallas temperature sensor
    float lastTemp;
    float currentTemp;

  public:
    TempSensor(int pin){
      this->pin = pin;
      owb = new OneWire(pin);
      dst = new DallasTemperature(owb);
      dst->begin();
      dp("Temperature sensors found:");
      dpln(dst->getDeviceCount());
      lastTemp = getTemp();
    }

  bool tempChanged(){
    return lastTemp != currentTemp;
  }
    
  float getTemp(){
    dst->requestTemperatures();
    lastTemp = currentTemp;
    currentTemp = dst->getTempCByIndex(0); // assuming that there is just one sensor on the bus
    return currentTemp;
  }
};


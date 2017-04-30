#include "debug.h"

class Relay {
  private:
    bool state;
    int pin;

  public:
    Relay(int pin){
      this->pin = pin;
      pinMode(pin,OUTPUT);
      off();
    }
    
    void setState(bool newState){
      if (newState != state) {
        state = newState;
        dp("Relay @");
        dp(pin);
        dp(" switched to ");
        dpln(state);
      }
      digitalWrite(pin, state);
    }
  
    void on(){
      this->setState(true);
    }
  
    void off(){
      this->setState(false);
    }

    bool isOn(){
      return state;
    }

};


#pragma once
#include "debug.h"

#define KBD_SIZE        20
#define KBD_BUFF_SIZE   50

#define KBD_KEY_MECHANICAL  0
#define KBD_KEY_CAPACITIVE  1

struct KeyDef {
  byte type;
  byte pin1;
  byte pin2;
  byte code;
};

class Keyboard {
  public:
  Keyboard(int delay){
    buffLen = 0;
    buffStart = 0;
    buffEnd = 0;
    keyCount = 0;
    combKeyCount = 0;
    this->delay = delay;
    isDelay = true;
    delayTimeEnd = millis();
  }

  void check(){
    bool done = false;
    byte i;

    if (isDelay && delayTimeEnd < millis())
      isDelay = false;

    if (isDelay) // don't read keyboard if in delay phase
      return;
     
    // check combined keys
    i = 0;
    while (!done && i < combKeyCount) {
      if (digitalRead(combKeys[i].pin1) && digitalRead(combKeys[i].pin2)){
        done = true;
        push(combKeys[i].code);
      }
      i++;
    }
    
    // check single keys
    i = 0;
    while (!done && i < keyCount) {
      if (digitalRead(keys[i].pin1)){
        done = true;
        push(keys[i].code);
      }
      i++;
    }
        
    
  }

  byte read(){
    if (available() == 0){
      return 0;
    }
    
    byte ret = buff[buffStart];
    buffStart++;
    buffLen--;
    if (buffStart >= KBD_BUFF_SIZE)
      buffStart = 0;
    dp("KBD Read ");  
    dpln(ret);
      
    return ret;
    
  }

  void push(byte k){
    dp("KBD Push ");
    dpln(k);
      
    buff[buffEnd] = k;
    buffEnd++;
    buffLen++;
    if (buffEnd >= KBD_BUFF_SIZE)
      buffEnd = 0;
    isDelay = true;  
    delayTimeEnd = millis()+delay;
  }


  byte available(){
    return buffLen;
  }

  bool registerKey(byte pin, byte code, byte type){
    keys[keyCount].pin1 = pin;
    keys[keyCount].type = type;
    keys[keyCount].code = code;    
    keyCount++;
  }

  bool registerKey(byte pin1, byte pin2, byte code, byte type){
    combKeys[combKeyCount].pin1 = pin1;
    combKeys[combKeyCount].pin2 = pin2;
    combKeys[combKeyCount].type = type;
    combKeys[combKeyCount].code = code;    
    combKeyCount++;
  }
  

  private:
  char buff[KBD_BUFF_SIZE];
  int buffLen;
  int buffStart;
  int buffEnd;
  int delay;
  bool isDelay;
  unsigned long delayTimeEnd;

  KeyDef keys[KBD_SIZE];
  byte keyCount;
  KeyDef combKeys[KBD_SIZE];
  byte combKeyCount;

  
};


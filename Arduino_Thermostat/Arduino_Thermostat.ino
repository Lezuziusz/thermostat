#include "debug.h"
#include <LiquidCrystal.h>
#include "Button.h"
#include "Relay.h"
#include "SensorAsync.h"
#include "Config.h"
#include "Keyboard.h"

#define VERSION "v1.4"

//############ HW CONFIG
// pin connected to temperature sensor
const byte TEMPS_1 = 7;

// Relays
const byte REL_1 = 8;

// Buttons
const byte BTN_LEFT = A1;
const byte BTN_RIGHT = A2;

// LCD display pins
const byte LCD_RS = 12;
const byte LCD_E = 11;
const byte LCD_D4 = 5;
const byte LCD_D5 = 4;
const byte LCD_D6 = 3;
const byte LCD_D7 = 2;

//############ SW CONFIG
float tempDelta = 0.1;   // when temperature is configured it will increase/decreate by this value on a single push

// LCD display
LiquidCrystal *lcd;

// Temperature sensors
TempSensor *temps1;

// Relays
Relay *relay1;      // relay 1

// Keyboard
#define KBD_CODE_LEFT         1
#define KBD_CODE_RIGHT        2
#define KBD_CODE_SELECT       11

#define KEYB_OFF_DELAY_LONG   500
#define KEYB_OFF_DELAY_SHORT  200
Keyboard kbd(250);

// 
byte mode = 0;
unsigned long modeEnd = 0;

#define MODE_DURATION     10000

#define MODE_NORMAL       0
#define MODE_CONFIG_TEMP  1
#define MODE_CONFIG_OP    2

bool blinkPhase = true;
int blinkOff = 150;
int blinkOn = 250;
unsigned long blinkStateEnd = millis();
float temp1 = 0;
float oldtemp1 = 0;

// backslash character for animation - not available in standard char-set
byte bkslash[8] = {
  B00000,
  B10000,
  B01000,
  B00100,
  B00010,
  B00001,
  B00000,
};

// characters for animantion - char(0) is backslash
char anim[] = {'-',0,'|','/'};
byte anim1State = 0;
unsigned long anim1StateEnd = millis();
#define ANIM_STATE_COUNT  4
#define ANIM_DURATION     200

// data structure contianing configuration
ConfigEE cfg;

void setup(void) {
  Serial.begin(115200);
  Serial.print("Thermostat ");
  Serial.println(VERSION);

  // register single keys
  kbd.registerKey(BTN_LEFT, KBD_CODE_LEFT, KBD_KEY_CAPACITIVE);
  kbd.registerKey(BTN_RIGHT, KBD_CODE_RIGHT, KBD_KEY_CAPACITIVE);

  // register combined keys
  kbd.registerKey(BTN_LEFT, BTN_RIGHT, KBD_CODE_SELECT, KBD_KEY_CAPACITIVE);

  // init display
  lcd = new LiquidCrystal(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
  lcd->createChar(0, bkslash);  // add backslah char as char(0)
  
  temps1 = new TempSensor(TEMPS_1);
  relay1 = new Relay(REL_1);
    
  lcd->begin(16, 2);
  
  lcd->setCursor(0, 0);
  lcd->print(F("Thermostat"));
  lcd->setCursor(0, 1);
  lcd->print(VERSION);
  delay(2000);
  lcd->clear();
  
  if (!cfg.read()){
    dpln(F("Failed to load configuration"));
    cfg.setDefaults();
    cfg.write();
  }
    
  cfg.print();  
}

void loop(void) {
    
  temp1 = temps1->getTemp();
  //dpln(btnLeft->isDown());
  //dpln(btnRight->isDown());

  // following block is only to decrease display rate durig debug
  #ifdef DEBUG_MODE
  if (temp1 != oldtemp1){
    dp("Temp=");
    dpln(temp1);
    oldtemp1 = temp1;
  }
  #endif

  // animate running operation
  if (relay1->isOn()) 
    animateOp1();

  // calculate blink ON/OFF state --------------
  if (millis() > blinkStateEnd){
    if (blinkPhase){
      blinkStateEnd = millis()+blinkOff;
    } else 
      blinkStateEnd = millis()+blinkOn;  
    blinkPhase = !blinkPhase;
  }

  // display operation mode: heating/cooling
  lcd->setCursor(0, 0);
  if (mode == MODE_CONFIG_OP && !blinkPhase)
    lcd->print("       : ");
  else  
    if (cfg.data.cooling)
      lcd->print("Cooling: ");
    else
      lcd->print("Heating: ");

  // display configured temperature
  lcd->setCursor(9, 0);
  if (mode == MODE_CONFIG_TEMP && !blinkPhase)
    lcd->print("     ");
  else
    lcd->print(cfg.data.temp);

  // display degrees C
  lcd->setCursor(14, 0);
  lcd->print(char(223));
  lcd->print("C");

  // display ccurrent temperature
  lcd->setCursor(3, 1);
  lcd->print("Temp: ");
  lcd->print(temp1);
  lcd->print(char(223));
  lcd->print("C");

  // OPERATE RELAY1 ------------------------------
  if (cfg.data.cooling) {                       // if operation is COOLING
    if (relay1->isOn()) {                       // if cooling is ON
      if (temp1 < cfg.data.temp-cfg.data.eps){  // then keep it on until temperature is lower than SET minus EPS
        relay1->off();                          // then turn it OFF
        clearOp1();
      }
        
    } else {                                        // if cooling is OFF
      if (temp1 > cfg.data.temp+cfg.data.eps)   // and current temperature is higer than SET plus EPS
        relay1->on();                           // then turn cooling ON
    }    
  }  else {                                     // if operation is HEATING
    if (relay1->isOn()) {                       // if heating is ON
      if (temp1 > cfg.data.temp+cfg.data.eps){  // then keep it ON until temperature is higher than SET plus EPS
        relay1->off();                          // then turn it OFF
        clearOp1();
      }
        
    } else {                                        // if heating is OFF
      if (temp1 < cfg.data.temp-cfg.data.eps)   // and current temperature is lower than SET minus EPS
        relay1->on();                           // then turn heating ON
    }
  }

  // manage keyboard ------------------------------
  kbd.check();
  byte key = kbd.read();
  if (key > 0){
    dp("Keyboard ");
    dpln(key);
  }

  switch (key) {
    case KBD_CODE_SELECT: // both LEFT & RIGHT pushed
      switch (mode) {
        case MODE_NORMAL:
            dpln("Set TEMP start");
            mode = MODE_CONFIG_TEMP;
          break;
        case MODE_CONFIG_TEMP:
            dpln(F("Set OP start"));
            mode = MODE_CONFIG_OP;
          break;
        case MODE_CONFIG_OP:
          dpln(F("Set TEMP start"));
          mode = MODE_CONFIG_TEMP;
          break;
      }
      modeEnd = millis()+MODE_DURATION;
      break;
      
    case KBD_CODE_LEFT:
      switch (mode) {
        case MODE_CONFIG_TEMP:
          cfg.data.temp -= tempDelta;
          dp("-");
          break;
        case MODE_CONFIG_OP:
          cfg.data.cooling = !cfg.data.cooling;
          break;
      }
      break;
      
    case KBD_CODE_RIGHT:
      switch (mode) {
        case MODE_CONFIG_TEMP:
          cfg.data.temp += tempDelta;
          dp("+");
          break;
        case MODE_CONFIG_OP:
          cfg.data.cooling = !cfg.data.cooling;
          break;
      }    
      break;
      
  }

  if (mode != MODE_NORMAL && millis() > modeEnd){
    mode = MODE_NORMAL;
    dpln(F("Set end"));
    cfg.write();
    cfg.print();
  }

  //delay(400);

}

void clearOp1(){
  lcd->setCursor(0,1);
  lcd->write(' ');  
}

void animateOp1(){
  if (millis() > anim1StateEnd){
    anim1StateEnd = millis() + ANIM_DURATION;
    lcd->setCursor(0,1);
    lcd->write(anim[anim1State]);
    anim1State++;
    if (anim1State >= ANIM_STATE_COUNT) 
      anim1State = 0;
  }
}



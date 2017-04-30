#include "debug.h"
#include <OneWire.h>
#include <LiquidCrystal.h>
#include "Button.h"
#include "Relay.h"
#include "Sensor.h"
#include "Config.h"

#define VERSION "v1.0"

//############ HW CONFIG
// pin connected to temperature sensor DS18S20
const byte TEMPS_1 = 7;

// Relays
const byte REL_1 = 8;

// Buttons
const byte BTN_DOWN = A1;
const byte BTN_UP = A2;
const byte BTN_ENTER = A6;
const byte BTN_EXIT = A7;

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

Button *btnDown;
Button *btnUp;

// Relays
Relay *relay1;      // relay 1

// 
byte mode = 0;
unsigned long modeEnd = 0;

#define MODE_DURATION     10000

#define MODE_NORMAL       0
#define MODE_CONFIG_TEMP  1
#define MODE_CONFIG_OP    2

bool blinkPhase = true;
int blinkOff = 500;
int blinkOn = 500;
unsigned long blinkStateEnd = millis();
float temp1 = 0;
float oldtemp1 = 0;

ConfigEE cfg;

void setup(void) {
  Serial.begin(115200);
  lcd = new LiquidCrystal(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
  temps1 = new TempSensor(TEMPS_1);
  btnDown = new Button(BTN_DOWN);
  btnUp = new Button(BTN_UP);
  relay1 = new Relay(REL_1);
  lcd->begin(16, 2);
  
  lcd->setCursor(0, 0);
  lcd->print("Thermostat");
  lcd->setCursor(0, 1);
  lcd->print(VERSION);
  delay(2000);
  
  if (!cfg.read()){
    dpln("Failed to load configuration");
    cfg.setDefaults();
    cfg.write();
  }
    
  cfg.print();  
}

void loop(void) {
  temp1 = temps1->getTemp();

  // following block is only to decrease display rate durig debug
  #ifdef DEBUG_MODE
  if (temp1 != oldtemp1){
    dp("Temp=");
    dpln(temp1);
    oldtemp1 = temp1;
  }
  #endif

  // calculate blink ON/OFF state --------------
  if (millis() > blinkStateEnd){
    if (blinkPhase){
      blinkStateEnd = millis()+blinkOff;
    } else 
      blinkStateEnd = millis()+blinkOn;  
    blinkPhase = !blinkPhase;
  }

  // display texts -----------------------------
  lcd->setCursor(0, 0);
  if (mode == 2 && !blinkPhase)
    lcd->print("     to: ");
  else  
    if (cfg.data.cooling)
      lcd->print("Cool to: ");
    else
      lcd->print("Heat to: ");

  lcd->setCursor(9, 0);
  if (mode == 1 && !blinkPhase)
    lcd->print("     ");
  else
    lcd->print(cfg.data.temp);

  lcd->setCursor(14, 0);
  lcd->print(char(223));
  lcd->print("C");
  
  lcd->setCursor(3, 1);
  lcd->print("Temp: ");
  lcd->print(temp1);
  lcd->print(char(223));
  lcd->print("C");

  // OPERATE RELAY1 ------------------------------
  if (cfg.data.cooling) {                       // if operation is COOLING
    if (relay1->isOn()) {                       // if cooling is ON
      if (temp1 < cfg.data.temp-cfg.data.eps)   // then keep it on until temperature is lower than SET minus EPS
        relay1->off();                          // then turn it OFF
        
    } else {                                        // if cooling is OFF
      if (temp1 > cfg.data.temp+cfg.data.eps)   // and current temperature is higer than SET plus EPS
        relay1->on();                           // then turn cooling ON
    }    
  }  else {                                     // if operation is HEATING
    if (relay1->isOn()) {                         // if heating is ON
      if (temp1 > cfg.data.temp+cfg.data.eps)   // then keep it ON until temperature is higher than SET plus EPS
        relay1->off();                          // then turn it OFF
        
    } else {                                        // if heating is OFF
      if (temp1 < cfg.data.temp-cfg.data.eps)   // and current temperature is lower than SET minus EPS
        relay1->on();                           // then turn heating ON
    }
  }

  // manage buttons ------------------------------
  // check if any of buttons has been released
  bool b1 = btnDown->check(LOW);
  bool b2 = btnUp->check(LOW);

  if (!b1 && !b2) 
    switch (mode) {
      modeEnd = millis()+MODE_DURATION;
      case MODE_NORMAL:
          dpln("Set TEMP start");
          mode = MODE_CONFIG_TEMP;
        break;
      case MODE_CONFIG_TEMP:
          dpln("Set OP start");
          mode = MODE_CONFIG_OP;
        break;
      case MODE_CONFIG_OP:
        dpln("Set TEMP start");
        mode = MODE_CONFIG_TEMP;
        break;
    }

  if (mode != MODE_NORMAL && millis() > modeEnd){
    mode = MODE_NORMAL;
    dpln("Set end");
    cfg.write();
    cfg.print();
  }

  if (btnUp->isDown() || btnDown->isDown()){
    switch (mode) {
      case MODE_CONFIG_TEMP: 
        if (btnDown->isDown()) {
          cfg.data.temp -= tempDelta;
          dp("-");
        }
        
        if (btnUp->isDown()) {
          cfg.data.temp += tempDelta;
          dp("+");
        }
        
        break;
      case MODE_CONFIG_OP:
          cfg.data.cooling = !cfg.data.cooling;
        break;
      
    }
    modeEnd = millis()+MODE_DURATION;
  }

  delay(100);

}



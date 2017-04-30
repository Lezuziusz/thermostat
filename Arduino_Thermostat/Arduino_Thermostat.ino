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

float tempDelta = 0.1;

// LCD display
LiquidCrystal *lcd;

// Temperature sensors
TempSensor *temps1;

Button *btnDown;
Button *btnUp;

// Relays
Relay *relay1;      // relay 1

// 
bool modeSetTemp = false;
unsigned long modeSetTempEnd = 0;

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
  
  if (temp1 != oldtemp1){
    dp("Temp=");
    dpln(temp1);
    oldtemp1 = temp1;
  }

  if (millis() > blinkStateEnd){
    if (blinkPhase){
      blinkStateEnd = millis()+blinkOff;
    } else 
      blinkStateEnd = millis()+blinkOn;  
    blinkPhase = !blinkPhase;
  }

  lcd->setCursor(0, 0);
  if (cfg.data.cooling)
    lcd->print("Cool to: ");
  else
    lcd->print("Heat to: ");

  lcd->setCursor(9, 0);
  if (modeSetTemp && !blinkPhase)
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

  if (cfg.data.cooling) {
    if (cfg.data.temp+cfg.data.eps < temp1) { // current temperature is outside higher limit
      //dpln("Cooling - relay ON");
      relay1->on();
    } else {
      //dpln("Cooling - relay OFF");
      relay1->off();
    }
  }

  bool b1 = btnDown->check(LOW);
  bool b2 = btnUp->check(LOW);

  if (!modeSetTemp){
    if (!b1 && !b2){
      dpln("SetTemp START");
      modeSetTemp = true;
      modeSetTempEnd = millis()+10000;
    }
  } else {
    if (millis() > modeSetTempEnd){
      modeSetTemp = false;
      dpln("SetTemp END");
      cfg.write();
    }
      
    if (btnDown->isDown()) {
      cfg.data.temp -= tempDelta;
      modeSetTempEnd = millis()+3000;
      dp("-");
    }
    
    if (btnUp->isDown()) {
      cfg.data.temp += tempDelta;
      modeSetTempEnd = millis()+3000;
      dp("+");
    }
    
  }

  delay(100);

}



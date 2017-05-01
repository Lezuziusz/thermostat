# Digital thermostat (Arduino + DS18B20 + display + buttons)

Complete thermostat solution :-)
* extensible for more sensors & relays
* configurable cooling/heating mode
* configurable temperature
* active relay is animated on the screen + LED can be used to show activity
* relay used is good for switching up to 100VDC, but intended operation uses 24VDC + contactor (e.g. Moeller) to manage high-power devices/motors

# Operation
1. Press & release both buttons at the same time to enter configuration mode
1. Press & release both buttons to select what to configure: temperature or operation. Item being configured is blinking.
1. Wait 10s to exit configuration mode. Changes are automatically saved to EEPROM.

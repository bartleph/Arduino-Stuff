/*
rtc_8segement
Real Time Clock with 8 Digit Segment-Display
Needed Hardware:-

An Arduino or compatible board like:
8 Digit Segment-Display with MAX7219
DS1307 RTC Breakout Board - Kit

Created by:
05/17/2012 by Marcel Hauri, http://m83.ch
Modified by:
02/05/2018 by Paul Bartlett, www.github.com/bartleph

Display shows time as HH-MM-SS and at every 10 sec interval switches to date as DD-MM-YY for 10 secs
*/


#include <LedControl.h>
#include <Wire.h>
#include "RTClib.h"

// SDA: A5
// SCL: A4
RTC_DS1307 RTC;

// PIN 10: CS
// PIN 11: CLK
// PIN 12: DIN
LedControl lc=LedControl(12,11,10,1);

void setup() {
  lc.shutdown(0,false);
  lc.setIntensity(0,1);
  lc.clearDisplay(0);
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  
  if (! RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}

void printLCD(int v, int a, int b) {
    int ones;
    int tens;
    
    ones=v%10;
    v=v/10;
    tens=v%10;

    lc.setDigit(0,a,(byte)ones,false);
    lc.setDigit(0,b,(byte)tens,false);
}

void loop() {
  DateTime now = RTC.now();
  
  int sensorValue = analogRead(A0);
  int val = (sensorValue/100)+1;
  lc.setIntensity(0,val);
  if (now.second() == 10 or now.second()==30 or now.second()==50)
   
  {
  printLCD(now.year(), 0, 1);
  lc.setChar(0,2,'-',false);
  printLCD(now.month(), 3, 4);  
  lc.setChar(0,5,'-',false);
  printLCD(now.day(), 6, 7); 
  delay(10000);
  
  
  }
  printLCD(now.second(), 0, 1);
  lc.setChar(0,2,'-',false);
  printLCD(now.minute(), 3, 4);  
  lc.setChar(0,5,'-',false);
  printLCD(now.hour(), 6, 7); 
  delay(10);
  
}

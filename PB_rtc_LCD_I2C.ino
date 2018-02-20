/*
 * A better Clock and LCD Solution.
 *LCD Clock using a DS1307 RTC and LCD connected via I2C and Wire lib
 Paul Bartlett Feb 2018
 
 */
#include <Wire.h>
#include "RTClib.h"
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#define I2C_ADDR    0x3F  // Define I2C Address for the LCD//

#define Rs_pin  0
#define Rw_pin  1
#define En_pin  2
#define BL_pin  3
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

#define  LED_OFF  1
#define  LED_ON  0

/* Config LCD */
LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);
RTC_DS1307 rtclock;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {
  lcd.begin (16,2);  // initialize the lcd 
// Switch on the backlight
  lcd.setBacklightPin(BL_pin,NEGATIVE);
  lcd.setBacklight(LED_ON);
  lcd.clear();
   Serial.begin(9600);
  if (! rtclock.begin()) {
    lcd.print("Couldn't find rtclock");
    while (1);
  }

  if (! rtclock.isrunning()) {
    lcd.print("rtclock is NOT running!");
    // following line sets the RTC to the date & time of this sketch
    rtclock.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // rtclock.adjust(DateTime(2018, 2, 20, 3, 0, 0));
  }
}

void loop () {
    DateTime now = rtclock.now();
    char dateBuffer[12];
    lcd.setCursor(3,0); //Start at character 0 on line 1
    sprintf(dateBuffer,"%02u-%02u-%04u ",now.day(),now.month(),now.year());
    lcd.print(dateBuffer);
    lcd.setCursor(4,1); //Second Line
    sprintf(dateBuffer,"%02u:%02u:%02u ",now.hour(),now.minute(),now.second());
    lcd.print(dateBuffer);
  
    delay(1000);
}



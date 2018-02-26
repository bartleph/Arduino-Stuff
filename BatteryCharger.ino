// AA and AAA charger
// Could use for others with different charge currents
// i.e. different emitter resistors.
// 
// LiquidCrystal library used here is not the original
// download an enhanced version from
// https://playground.arduino.cc/Main/LiquidCrystal
// LiquidCrystal_1.zip allows SPI operation.
//
// V1.00
// Copyright John Main
// Free for non-commercial use.
//
#include <SPI.h>
#include <LiquidCrystal.h> // Enhanced for SPI operation. 

#define START_PIN 12
#define RLED_PIN A4
#define GLED_PIN A5

#define VBATT_PIN A0
#define CHRG_PIN 2
#define HIGH_PIN 3
#define SPI_SS_PIN 10

#define CHARGE_ON  pinMode(CHRG_PIN,OUTPUT);digitalWrite(CHRG_PIN,LOW);
#define CHARGE_OFF pinMode(CHRG_PIN,INPUT);
#define HIGH_ON    pinMode(HIGH_PIN,OUTPUT);digitalWrite(HIGH_PIN,LOW);
#define HIGH_OFF   pinMode(HIGH_PIN,INPUT);
#define RLED_ON    digitalWrite(RLED_PIN,HIGH);
#define RLED_OFF   digitalWrite(RLED_PIN,LOW);
#define GLED_ON    digitalWrite(GLED_PIN,HIGH);
#define GLED_OFF   digitalWrite(GLED_PIN,LOW);
#define SPC        Serial.print(' ');

#define VBATMAX 1.60 // Maximum and minimum acceptable battery.
#define VBATMIN 0.75 // voltage to allow charging to start.
#define VREFANA 2.36 // Reference voltage (AREF). Measure this with DMM.
#define CHRG_ms 30000 // Time period of normal charging time (ms).
#define HIGH_ms 30000 // Time period of normal higher power charging time (ms).

#define PREV_ADC 10  // Number of nearly same ADC readings to declare done.

typedef enum { IDLE, BATFND, CHARGE, HIGH_CHARGE, TEST,
               FINISHED, FAIL, WAIT_START } state_t;
               
enum { BATTFULL, BATTAVGSTABLE, BATTBAD, BATTNORISE };

// Initialize for SPI with sspin, also known as RCLK or LATCH.
// For the lcd other pins are the standard SPI pins.
LiquidCrystal lcd(SPI_SS_PIN);

///////////////////////////////////////////////////
void setup() {

   pinMode(VBATT_PIN,INPUT);
   pinMode(START_PIN,INPUT_PULLUP);
  
   pinMode(CHRG_PIN,OUTPUT);
   pinMode(HIGH_PIN,OUTPUT);
   pinMode(GLED_PIN,OUTPUT);
   pinMode(RLED_PIN,OUTPUT);
  
   CHARGE_OFF
   HIGH_OFF
   RLED_OFF
   GLED_OFF
   analogReference(EXTERNAL);
  
   // set up the LCD's number of columns and rows: 
   lcd.begin(16, 2);
  
  
    
   Serial.begin(57600);
   Serial.println("Battery Charger ");
    showStartVolts();
}

///////////////////////////////////////////////////
// Print start volts to LCD & serial(for reference).
void showStartVolts(void) {
  
  float v = getVana( analogRead(VBATT_PIN) ) ;
  
  lcd.setCursor(11, 0);    
  lcd.print( v,3 );
  Serial.print("\nStart volts: ");
  Serial.println( v,3 );
}

///////////////////////////////////////////////////
void reason(uint8_t c) {
   Serial.print("End reason: ");
   switch(c) {
      case BATTFULL       : Serial.println("Max volts."); break;
      case BATTAVGSTABLE  : Serial.println("No avg change."); break;
      case BATTBAD        : Serial.println("Out of range.");
      case BATTNORISE     : Serial.println("No Hchrg rise.");
   }
}
///////////////////////////////////////////////////
void loop() {
  static uint32_t loop_time = 0;
  
  if (millis()-loop_time>250) {  // No need execute too fast
     action();
     loop_time = millis(); 
  }
}

///////////////////////////////////////////////////
float getVana(uint16_t adc) { 
    return adc * VREFANA / 1024;
}

///////////////////////////////////////////////////
uint8_t checkBattBad(float Vana) { 
   if ( Vana<VBATMIN || Vana>VBATMAX ) return 1;
   return 0;
}

///////////////////////////////////////////////////
uint8_t checkBattFinished(float Vana) { 
   if ( Vana>=VBATMAX-0.1 ) return 1;
   return 0;
}

///////////////////////////////////////////////////
void showSerialElapsedTime(uint32_t r_time) { 
   Serial.print("Time:");
   Serial.print(r_time/1000/60); 
   Serial.println(" mins.");   
}

///////////////////////////////////////////////////
void showState(state_t state) {
  
   // This state has no text, to leave FINISHED or FAIL on LCD.
   if (state == WAIT_START) return; 
  
   // Show current state 
   lcd.setCursor(0, 0);
   lcd.print("           "); // 11 chars clear debug area.
   lcd.setCursor(0, 0); 
   switch( state) {
     case IDLE : lcd.print("IDLE");break;
     case BATFND : lcd.print("BATFND");break;
     case CHARGE : lcd.print("CHRG");break;
     case HIGH_CHARGE : lcd.print("H-CHRG");break;
     case TEST : lcd.print("TEST");break;
     case FINISHED : lcd.print  ("FINSHD");break;
     case FAIL : lcd.print("FAIL");break;
//     case WAIT_START : lcd.print("WAIT");break;
    default: break;
   }
}

///////////////////////////////////////////////////
void showTime(uint32_t timeVal_ms) { // could use sprinf - this = less memory.
  uint8_t lcdpos;
  uint16_t tmin,tsec;
  uint32_t t;
    
    t = timeVal_ms;
    tmin = (t/1000/60); // Could use sprintf but smaller code using:

    if (tmin >= 1) { // Print minutes since reset
       lcdpos = 12;
       if (tmin>99) lcdpos = 10;
          else if (tmin>9) lcdpos = 11;       
       lcd.setCursor(lcdpos, 1); 
       
       lcd.print(tmin);
       lcd.print(':');
    }
    
    // Print seconds since reset.
    tsec = (t/1000) % 60;
    
    lcd.setCursor(14, 1);
    if (tsec<10) 
       lcd.print('0'); 
    lcd.print(tsec);
}

///////////////////////////////////////////////////
// -1 resets the store
//
uint8_t test_avg(uint16_t adc) {
uint8_t i,match,allmatch;
static uint16_t prevADC[PREV_ADC];
uint16_t avg;

   if (adc==-1) {
      for(i=0;i<PREV_ADC;i++) prevADC[i]=0;
      return;
   }
  
   // Get average value 
      avg = 0;
      Serial.print("AVG:");  
      for (i=0;i<PREV_ADC;i++) {
         Serial.print(prevADC[i]); SPC;
         avg+=prevADC[i];   
      }
      avg += 5;  // =0.5 after division by 10
      avg /=10;
      Serial.print("Avg ");Serial.println(avg);
      
      // If any are zero then not filled: set avg zero to stop err. match
      for (i=0;i<PREV_ADC;i++) {
          if (prevADC[i]==0) { 
             avg = 0;
             Serial.println("Avg zet ZERO");
             break;
          }
      }
      
      // Check if has not increased over last n readings - if so exit.      
      allmatch = 1;   
     
      for (i=0;i<PREV_ADC;i++) {
         match = 0;
         // Detect close matches: lowChrgADC+/-2 (+/-4.88mV)
         if ( avg-1 <= prevADC[i] && prevADC[i] <= avg+1 ) match=1;
         
         // Here if the value is one of 3: lowChrgADC+/-1, then match is high
         if (!match) { allmatch = 0; break; }
      }
      
      // Update rolling store. // Array indices 0 ~ (n-1) shift.
      for (i=0;i<PREV_ADC-1;i++) prevADC[i]=prevADC[i+1]; 
      prevADC[PREV_ADC-1] = adc;
  
      // allmatch is 1 if all have matched for all elements.
      if (allmatch && avg!=0) { // Zero is a special case.
         reason(BATTAVGSTABLE);
         return 1; // All the same so indicate finished.
      }
      return 0;
}



///////////////////////////////////////////////////
void action(void) {
   static uint32_t s_time = millis();
   static uint32_t r_time = s_time; // Relative, start times.
   static uint32_t timewas = 0, timewas_state=r_time;
   static state_t state = IDLE; 
   static uint16_t lowChrgADC=0;
   static uint8_t timer_on=1;
   uint16_t adc=0;
   float Vana;
   static uint8_t done_once = 0;

   if (!done_once) {
      test_avg(-1); // Reset store.
      done_once=1;
   }
    
   r_time = millis() - s_time; // Use time relative to start
   if (timer_on) showTime(r_time); // Time since start: to LCD.

   adc = analogRead(VBATT_PIN);
   lcd.setCursor(0, 1);
   lcd.print("Vb:");
   lcd.print(Vana=getVana(adc),3);
    
   switch( state) {
      
   case IDLE :  
       state = BATFND;
       Serial.println("IDLE>BATFND"); 

       CHARGE_ON   // Start charge to detect battery.
       HIGH_OFF
       
       RLED_OFF
       GLED_OFF

       showSerialElapsedTime(r_time);
       
       break;
        
   case BATFND : // Battery ok?
       Serial.println("BATFND>CHARGE");
        
       if ( checkBattBad(Vana) ) {
          state = FAIL; // Battery is dead or not present.
          Serial.println("FAIL in BATFND"); 
       } else if (Vana>=VBATMIN && Vana<=VBATMAX) {
           state = CHARGE;
       }   
       break;
       
   case CHARGE : 
      if ( checkBattBad(Vana) )      { reason(BATTBAD) ; state = FAIL; }   
      if ( checkBattFinished(Vana) ) { reason(BATTFULL); state = FINISHED; }
      else if (r_time - timewas_state>CHRG_ms) {   
         Serial.println("CHARGE>HCHARGE"); 
          
         lowChrgADC = adc; // Store value just before high charge time.          
           
         HIGH_ON
         state = HIGH_CHARGE;   
         timewas_state = r_time; 
      }        
      break;
       
   case HIGH_CHARGE : 
      if ( checkBattBad(Vana) )      { reason(BATTBAD) ; state = FAIL; }   
      if ( checkBattFinished(Vana) ) { reason(BATTFULL); state = FINISHED; }    
      else if ( r_time - timewas_state>HIGH_ms) {
         Serial.println("HCHARGE>TEST"); 
         state = TEST;
         timewas_state = r_time; 
      }
      break;  

   case TEST : 
      Serial.println("TEST>IDLE/FINISHED");
      Serial.println("Cur adc,lowChrgADC");
      Serial.print(adc);       SPC; Serial.println(getVana(adc),3); 
      Serial.print(lowChrgADC);SPC;Serial.println(getVana(lowChrgADC),3); 
       
      if ( adc > (lowChrgADC+4) ) {  // Gone up by > (2.5/1024)*4=9.77mV    
         state= IDLE; 
      } else { // No rise so batery charge maxed out.   
         reason(BATTNORISE);       
         state = FINISHED;
      }  
      
      if ( test_avg(lowChrgADC) ) state = FINISHED;
            
      break; 
       
   case FAIL : // Error condition so stop charging battery.
      Serial.println("FAIL");               
      CHARGE_OFF
      HIGH_OFF
      timer_on = 0;
      RLED_ON
      GLED_OFF
      state = WAIT_START;
      break;
      
   case FINISHED : 
      Serial.print("FINISHED:"); 
      Serial.println(Vana); 
      showSerialElapsedTime(r_time);
      
      CHARGE_OFF
      HIGH_OFF
      RLED_OFF      
      GLED_ON      
      timer_on = 0;
      state = WAIT_START;
      break;

   case WAIT_START :
      if ( digitalRead(START_PIN)==0) {
         state = IDLE;
         
         s_time = millis();
         r_time  = millis()-s_time;
         timewas_state = r_time;
                
         timer_on = 1;
         test_avg(-1); // Reset store.
         lcd.clear();
         showStartVolts();        
      }
      break;    
   }
   
   showState(state);
   
   // Show raw adc value
   lcd.setCursor(7, 0);    
   lcd.print(adc);lcd.print(' ');
}

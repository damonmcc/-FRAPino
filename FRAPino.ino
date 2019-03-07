/*
 * FRAPino: Control system for FRAP experiments
 * Damon McCullough, 2018
 */

#define PIN_HPLED 10  // Pin# of digital output to High-power LED trigger
#define PIN_BLLED 2  // Pin# of digital output to Baseline LED trigger
#define PIN_CAMTRIG 12  // Pin# of digital output to Camera trigger port
#define PIN_IRREC 11  // Pin# of IR receiver to digital PWM

#include "utilities.h"
#include "menu_setup.h"


void setup() {
  Serial.begin(115200);
  Serial.println("INIT LCD...");

  digitalWrite(LED_BUILTIN, HIGH);
  lcd.begin(16,2);
  lcd.setCursor(0, 0);
  lcd.print("FRAPino V0.2");
  lcd.setCursor(0, 1);
  lcd.print("__|_|_|_|______");
  delay(2000);
  
  pinMode(PIN_HPLED, OUTPUT);
  pinMode(PIN_BLLED, OUTPUT);
  pinMode(PIN_CAMTRIG, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("IR Receiver Button Decode"); 
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    translateIR(); 
    irrecv.resume(); // receive the next value
  }
  nav.poll();
}





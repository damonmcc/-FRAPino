// Delays around HP-LED bursts
// Units: milliseconds
int baselineWindow = 100;  //  0.1 seconds

// Total HP-LED trigger pulse length (ON + OFF) in us
// 0.2 - 6 ms
// 200 - 6000 us
float HPpulseLength = 6000;
// Duty cycle of pulse in %
float HPdutyCycle = 50;
// Number of pulses per burst (850)
int HPcount = 850;
// may need 240 ms (calculated in excel)
unsigned long HPcooldown = HPpulseLength*10;
// Will store last time HP-LED pin was updated
unsigned long HPledPreviousHIGHMicros = 0;
unsigned long HPledPreviousLOWMicros = 0;
unsigned long HPledWaitPreviousMicros = 0;
long HPledIntervalHigh = HPpulseLength*(HPdutyCycle/100);
long HPledIntervalLow = HPpulseLength - HPledIntervalHigh;
unsigned long HPcurrentMicros = 0;

// Used to set the HP-LED
int HPledState = LOW;   


// Total pulse length (ON + OFF) in us
// 1 - 100 Hz (FPS)
// 1000000 - 100000 us
// Default: 60 FPS
// 1000000*(1/60) us
float CAMpulseLength = 0.03219 * 1000000;
// Duty cycle of pulse in %
float CAMdutyCycle = 99;
//  capture window after burst: 5 mins - (baseline+burst)
// 1*1000 = 1 second
int captureWindow = (1*1000)-(baselineWindow + HPcount*HPpulseLength*1000);    
// Number of frames
int CAMframes = (1000*captureWindow)/(int) CAMpulseLength;

// Will store last time Camera pin was updated
unsigned long CAMPreviousHIGHMicros = 0;
unsigned long CAMPreviousLOWMicros = 0;
unsigned long CAMWaitPreviousMicros = 0;
long CAMIntervalHigh = CAMpulseLength*(CAMdutyCycle/100);
long CAMIntervalLow = CAMpulseLength - CAMIntervalHigh ;
unsigned long CAMcurrentMicros = 0;

// Used to set the Camera trigger
int CAMState = LOW; 

unsigned long currentMicros = 0;


/* waitMicro(dur)
 * Waits/delays for a given number of microseconds
 */
void waitMicro(int dur){
  unsigned long currentMicros = micros();
  unsigned long WaitPreviousMicros = currentMicros;
  while(currentMicros - WaitPreviousMicros < dur){
    currentMicros = micros();
  }
}

/* baseline(window)
 * Turn on baseline LED for a certain # of microseconds
 */
void baselineON(){
  digitalWrite(PIN_BLLED, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
}
void baselineOFF(){
  digitalWrite(PIN_BLLED, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}


/* burst(total, duty, count)
 * Sends burst of pulses to HPLED
 */
void burst(int count){
  int i = 0;
  HPledState = HIGH;
  HPcurrentMicros = micros();
  HPledPreviousHIGHMicros = HPcurrentMicros;
  digitalWrite(PIN_HPLED, HPledState);  // Start with turning ON
  while (i < count){
    //  Save the current time at the start of every loop
    HPcurrentMicros = micros();
    
    if(HPledState == LOW){ // Check to see if it's time to turn ON
      if (HPcurrentMicros - HPledPreviousLOWMicros >= HPledIntervalLow) {
        i++;
        if(i >= count){break;}
        //    Save the last time LED turned OFF
        HPledPreviousHIGHMicros = HPcurrentMicros;    
        HPledState = HIGH;
      }
    }
    else{ // Check to see if it's time to turn OFF
      if (HPcurrentMicros - HPledPreviousHIGHMicros >= HPledIntervalHigh) {
        //    Save the last time LED turned ON
        HPledPreviousLOWMicros = HPcurrentMicros;
        HPledState = LOW;
      }
    }  
    digitalWrite(PIN_HPLED, HPledState);
  }
}

/* capture(frames)
 * Sends burst of pulses to Camera trigger
 */
void capture(int frames){
  int i = 0;
  CAMState = HIGH;
  CAMcurrentMicros = micros();
  CAMPreviousHIGHMicros = CAMcurrentMicros;
  digitalWrite(PIN_CAMTRIG, CAMState);  // Start with turning ON
  while (i < frames){
    //  Save the current time at the start of every loop
    CAMcurrentMicros = micros();
    
    if(CAMState == LOW){ // Check to see if it's time to turn ON
      if (CAMcurrentMicros - CAMPreviousLOWMicros >= CAMIntervalLow) {
        i++;
        if(i >= frames){break;}
        //    Save the last time LED turned OFF
        CAMPreviousHIGHMicros = CAMcurrentMicros;    
        CAMState = HIGH;
      }
    }
    else{ // Check to see if it's time to turn OFF
      if (CAMcurrentMicros - CAMPreviousHIGHMicros >= CAMIntervalHigh) {
        //    Save the last time LED turned ON
        CAMPreviousLOWMicros = CAMcurrentMicros;
        CAMState = LOW;
      }
    }  
    digitalWrite(PIN_CAMTRIG, CAMState);
  }
}
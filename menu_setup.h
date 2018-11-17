#include <Arduino.h>
#include <menu.h>
//inputs
#include <menuIO/serialIn.h>
//outputs
#include <menuIO/liquidCrystalOut.h>
#include <menuIO/serialOut.h>

//include plugins
#include <plugin/cancelField.h>
#include <plugin/barField.h>
#include <IRremote.h>

IRrecv irrecv(PIN_IRREC);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'


// LCD /////////////////////////////////////////
#define RS 8
#define RW 3
#define EN 9
LiquidCrystal lcd(RS, RW, EN, 4, 5, 6, 7);
#define LEDPIN 13

using namespace Menu;

int test=55;
int ledCtrl=LOW;
serialIn serial(Serial);
char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
char buf1[]="0x11";
result runFRAP(eventMask e, prompt &item);
result runPEEK(eventMask e, prompt &item);

// Custom menu item to choose parameter settings
class confirmParams:public menu {
public:
  confirmParams(constMEM menuNodeShadow& shadow):menu(shadow) {}
  Used printTo(navRoot &root,bool sel,menuOut& out, idx_t idx,idx_t len,idx_t p) override {
    return idx<0?//idx will be -1 when printing a menu title or a valid index when printing as option
      menu::printTo(root,sel,out,idx,len,p)://when printing title
      out.printRaw((constText*)F("No"),len);//when printing as regular option
  }
};

// Custom menu item to confirm FRAP start
class confirmFRAP:public menu {
public:
  confirmFRAP(constMEM menuNodeShadow& shadow):menu(shadow) {}
  Used printTo(navRoot &root,bool sel,menuOut& out, idx_t idx,idx_t len,idx_t p) override {
    return idx<0?//idx will be -1 when printing a menu title or a valid index when printing as option
      menu::printTo(root,sel,out,idx,len,p)://when printing title
      out.printRaw((constText*)F("Start"),len);//when printing as regular option
  }
};

// Paramater menu for Baseline LED (HI_LED)
MENU(paramsBaseline, "BaseLine Params.",doNothing,noEvent,wrapStyle
  ,altFIELD(cancelField, baselineWindow,"Period:","ms",0,1000,10,0,doNothing,enterEvent,wrapStyle)//cancelable field (2 edit levels)
  ,EXIT("<Exit")
);

// Paramater menu for High Power Led (HI_LED)
MENU(paramsHPLED, "HP-LED Params.",doNothing,noEvent,wrapStyle
	,altFIELD(cancelField,HPcount,"#ofPulse:","",0,10000,100,10,doNothing,enterEvent,wrapStyle)//cancelable field (2 edit levels)
  ,EXIT("<Exit")
  ,altFIELD(cancelField, HPdutyCycle,"D.Cyc:","%",0,99,10,1,doNothing,enterEvent,wrapStyle)//cancelable field (2 edit levels)
	,altFIELD(cancelField, HPpulseLength,"PulseLen:","us",100,6000,10,0,doNothing,enterEvent,wrapStyle)//cancelable field (2 edit levels)
  ,altFIELD(cancelField, HPcooldown,"Cooldwn:","us",100,100000,1000,100,doNothing,enterEvent,wrapStyle)//cancelable field (2 edit levels)
);

// Paramater menu for Camera
MENU(paramsCamera, "Camera Params.",doNothing,noEvent,wrapStyle
  ,altFIELD(cancelField, CAMframesBase,"Base Fr:","",0,10000,10,1,doNothing,enterEvent,wrapStyle)//cancelable field (2 edit levels)
  ,altFIELD(cancelField, CAMframesRecov,"Recov Fr:","",0,100000,100,10,doNothing,enterEvent,wrapStyle)//cancelable field (2 edit levels)
  ,altFIELD(cancelField, CAMexposure,"Exp:0.0","",1,1000,10,1,doNothing,enterEvent,wrapStyle)//cancelable field (2 edit levels)
  ,EXIT("<Exit")
);

// altMenu for starting FRAP
altMENU(confirmFRAP, frapMenu," Start FRAP?",doNothing,noEvent,wrapStyle,(Menu::_menuData|Menu::_canNav)
  ,OP("Yes!", runFRAP,enterEvent)
  ,OP("PEEK!", runPEEK,enterEvent)
  ,EXIT("<Cancel surgery")
);
// altMenu for non-default parameter choices & FRAP starting
// TODO add camera params (delay after bleach, frame rate, etc.)
altMENU(confirmParams, paramMenu," Parameters",doNothing,noEvent,wrapStyle,(Menu::_menuData|Menu::_canNav)
	,SUBMENU(paramsHPLED)
  ,EXIT("<Exit")
  ,SUBMENU(frapMenu)
  ,SUBMENU(paramsCamera)
  ,SUBMENU(paramsBaseline)
);


// A main menu for FRAPulse control
MENU(mainMenu1," Use Defaults?",doNothing,noEvent,wrapStyle
	,SUBMENU(frapMenu)
	,SUBMENU(paramMenu)
);


//a menu using a plugin field
MENU(mainMenu,"Main menUE",doNothing,noEvent,wrapStyle
  ,BARFIELD(test,"Bar field","%",0,100,10,1,doNothing,noEvent,wrapStyle)//numeric field with a bar
  ,FIELD(test,"Original","%",0,100,10,1,doNothing,noEvent,wrapStyle)//normal numeric field (2 edit levels)
  ,FIELD(test,"O. Simple","%",0,100,1,0,doNothing,noEvent,wrapStyle)//normal numeric field (1 edit level)
  ,altFIELD(cancelField,test,"Cancelable","%",0,100,10,1,doNothing,enterEvent,wrapStyle)//cancelable field (2 edit levels)
  ,altFIELD(cancelField,test,"C. Simple","%",0,100,1,0,doNothing,enterEvent,wrapStyle)//cancelable field (1 edit level)
  ,EDIT("Hex",buf1,hexNr,doNothing,noEvent,noStyle)
  ,EXIT("<Exit")
);

#define MAX_DEPTH 3

MENU_OUTPUTS(out,MAX_DEPTH
  ,SERIAL_OUT(Serial)
  ,LIQUIDCRYSTAL_OUT(lcd,{0,0,16,2})
);

NAVROOT(nav,mainMenu1,MAX_DEPTH,serial,out);//the navigation root object


result runFRAP(eventMask e, prompt &item) {
  // capture((1000*baselineWindow)/(int)CAMpulseLength);
  // Frames based on 0.03219, or ~31 FPS
  // 1864 frames -> 59.94 seconds
  // 466 frames -> 14.99 seconds
  baselineON();
  lcd.clear();
  lcd.print("FRAP STEP:");
  lcd.setCursor(0, 1);
  lcd.print("Baseline...");
  capture(CAMframesBase);

  lcd.setCursor(0, 1);
  lcd.print("Cooling...");
  waitMicro(1000000); // wait 1 second...

  baselineOFF();
  lcd.clear();
  lcd.print("FRAP STEP:");
  lcd.setCursor(0, 1);
  lcd.print("Bleach!");
  burst(HPcount);
  digitalWrite(PIN_HPLED, LOW);

  lcd.setCursor(0, 1);
  lcd.print("Cooling...");
  delayMicroseconds(200000); // wait 2 seconds...

  baselineON();
  lcd.clear();
  lcd.print("FRAP STEP:");
  lcd.setCursor(0, 1);
  lcd.print("Recovery...");
  capture(CAMframesRecov);
  baselineOFF();
  nav.doNav(upCmd);
  return proceed;
}

// turn on HP-LED for thrice as long as the photobleach burst
// NOTE: TURN DOWN POWER BEFORE PEEKING
result runPEEK(eventMask e, prompt &item) {
  digitalWrite(LED_BUILTIN, HIGH);
  // waitMicro(HPcooldown);
  burst(HPcount);
  // waitMicro(HPcooldown);
  // nav.doNav(upCmd);
  digitalWrite(LED_BUILTIN, LOW);
  return proceed;
}


/* translateIR()
 * Takes action based on IR code received
 */
void translateIR() 
{
  delay(100); // Do not get immediate repeat
  switch(results.value){
  case 0xFFA25D: Serial.println("POWER"); break;
  case 0xFFE21D: Serial.println("FUNC/STOP");
    nav.doNav(escCmd);
    break;
  case 0xFF629D:Serial.println("VOL+");
    nav.doNav(upCmd);
    break;
  case 0xFF22DD: Serial.println("FAST BACK");
    // nav.doNav(escCmd);
    break;
  case 0xFF02FD: Serial.println("PAUSE");
    nav.doNav(enterCmd);
    break;
  case 0xFFC23D: Serial.println("FAST FORWARD");   break;
  case 0xFFE01F: Serial.println("DOWN");
    nav.doNav(downCmd);
    break;
  case 0xFFA857: Serial.println("VOL-");
    nav.doNav(downCmd);
    break;
  case 0xFF906F: Serial.println("UP");
    nav.doNav(upCmd);
    break;
  case 0xFF9867: Serial.println("EQ");    break;
  case 0xFFB04F: Serial.println("ST/REPT");    break;
  case 0xFF6897: Serial.println("0");    break;
  case 0xFF30CF: Serial.println("1");    break;
  case 0xFF18E7: Serial.println("2");    break;
  case 0xFF7A85: Serial.println("3");    break;
  case 0xFF10EF: Serial.println("4");    break;
  case 0xFF38C7: Serial.println("5");    break;
  case 0xFF5AA5: Serial.println("6");    break;
  case 0xFF42BD: Serial.println("7");    break;
  case 0xFF4AB5: Serial.println("8");    break;
  case 0xFF52AD: Serial.println("9");    break;
  case 0xFFFFFFFF: Serial.println("_REPEAT");break;  

  default: 
    Serial.println(" other button ");
  }// End Case
}

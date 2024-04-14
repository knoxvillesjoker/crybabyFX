/*
CryBaby FX Kit v1.0, written by Sean Maio (outcry27@gmail.com)
Originally Designed for use with the Adafruit Pro Trinket and Adafruit audio FX board

Updated January 2024 Cobra_shipwreck adding grenade counter, second grenade display and counter, and full/semi/burst triggering

Credit and many thanks to the writers of the numerous tutorials available on both the Arduino
and Adafruit websites, without which this project could never have been completed!

This now uses an arduino Micro, 2x 14segment ht16k33 backpacks, fxsoundboard, motorcycle light handlebar assembly for switches
https://www.adafruit.com/product/1911 (this is the display used)
https://www.adafruit.com/product/2220 (this is the fx soundboard)
https://store-usa.arduino.cc/products/arduino-micro (this is the arduino micro used)
https://www.adafruit.com/product/4981 (this is the ethernet screw terminal adapter I use)

Please be advised screw terminal or directly soldered wires are the best way to connect things.  Jumper wires are problematic.
And jumper wires are known to be problematic due to intermittant connection issues and strain issues worsen things.
It is recommended that you have spare parts for things due to con wear and board altering shorts.

If you are using an actual motorcycle light switch assembly ( you need one with buttons: Horn, Highbeam, light, hazard, left and right signal)
  1. Horn is the Fire Switch
  2. High beam is the Grenade Switch
  3. Hazard is the safety Switch
  4. Left is Semi Auto (default is burst fire)
  5. Right is Full auto
  6. Light Switch is used for the Mosfet for the airsoft portion and is a manual battery disconnect for the LIPO the airsoft gun uses

-Pin Map-
  Pro Trinket: (Micro pin noted in parenthesis)
  0. Serial RX, connect to Audio FX Serial TX (on Micro labeled D1/TX/PD3)
  1. Serial TX, connect to Audio FX Serial RX (on Micro labeled D0/RX/PD2)
  3. Fire Switch -> GND (on Micro D3 is SCL, so use D4/PD4)
  4. Safety toggle switch -> GND (on Micro  use D5/PC6)
  9. Muzzle LED, connect this to the GATE pin on the muzzle strobe MOSFET (on Micro use D9/PB5)
  8. Grenade -> GND (on Micro D8/PB4)
  7. Semi -> GND (on Micro D7/PE6)
  6. Full -> GND (on Micro D6/PD7)
  DigitalRead is used to detect momentary button presses a value of HIGH means pressed, a value of LOW means not pressed.

  Please note on the arduino Micro the UART(serial) has to be called using Serial1.blah.
  Due to the design there is not an easy way to test things with the serial monitor.
  The Arduino Micro uses the same chip as the Leonardo so if you use that board it wll have the same issue as well.
  And UART on the micro can only be used on the tx/rx pins, it will not work on any other pins.  I have verified this bit, but you are welcome to prove me wrong if you so desire.
 
  Audio FX board:
  TX. Connect to Trinket Pin 2 (these cross over so this goes to RX on Micro) (disconnect while uploading via USB as this is used for USB too)
  RX. Connect to Trinket Pin 1 (these cross over so this goes to TX on Micro)
  UG. Jumper to GND to enable UART mode. (this allows direct serial triggering using the Serial.print command set(less wires used))
 
  These are the invokations for the sound effects:
  Please note that the files are named specifically on the FX Sound Board to play when these are called serially in UART mode.
  1. This invokes the bootup sound: Serial.print("#2\n");
  2. This invokes the click sound:  Serial.print("#1\n");
  3. This invokes the firing sound: Serial.print("#3\n");
  4. This invokes the Grenade sound:Serial.print("#0\n");
  The naming of the files has to be T00.wav, T01.wav, T02.wav, T03.wav.
  To call the files you have to call them minus the preceeding zero otherwise it will only play file 0.   
  Also note that the files have to be 16-bit signed PCM at 22khz mono wav files.  No other settings will work or play.  Ogg files will not work as they cause a delay on looping sounds like the fire sfx.  Please note that these adjustments can be done in Audacity, a free and open source sound editing program.  There are youtube tutorials and writeups on how to edit sound files.  Just bear in mind edit or convert the files as few times as possible as the more you convert the file the more fidelity(audio quality) you lose.
*/

#include <Wire.h>
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

// pins used for Serial communication with the audio board
#define SFX_TX 1
#define SFX_RX 0
// This calls the LCD display for it to operate and it uses Digital Pins 2 and 3 for SCL/SDA on Micro, A4/A5 on Trinket Pro.
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, NULL);
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4(); //this is for the mg rounds
Adafruit_AlphaNum4 alpha5 = Adafruit_AlphaNum4(); //this is for the grenade rounds

// constants:
const int triggerPin = 5;     // the number of the firing trigger pin
const int safetyPin = 4; // pin for the "safety" toggle
const int fireSoundPin = 8; // pin for the firing sound
const int muzzlePin =  9;      // the number of the LED pin
const int startSoundPin = 9; // pin for the "boot-up" sound
const int clickSoundPin = 4; // pin for the bolt "click" sound
const int grenadeSoundPin = 5;
const int fireDelay = 50; // duration in milliseconds for each "shot"
// these are the additional pins
const int grenadePin = 8; // pin for the grenade sound
const int fullPin = 7; // pin for fire selection mode select
const int semiPin = 6; // other pin for fire selection mode select

// variables:
boolean safetyOn = false;
int triggerState = 0;         // variable for reading the trigger status
int safetyState = 0;          // variable for reading the safety switch status
int lastTriggerState = 0;     // previous state of the trigger
int lastSafetyState = 0;      // previous state of the safety switch
int ammoCount = 299;
String ammoDisp = String(ammoCount);

// these are for the additional pins
int grenadeCount = 200;  //default grenade ammo count
String grenadeDisp = String(grenadeCount);
int fireSelect = 0;
int lastgrenadetriggerState = 0;
int grenadetriggerState = 0;
int semiState = 0;
int fullState = 0;
boolean fullOn = false;
boolean semiOn = false;
int lastsemiState = 0;
int lastfullState = 0;
int lastburstState = 0 ;
int burstState = 0;
boolean burstOn = false;
int fullbuttonCount = 0;
int semibuttonCount = 0;

enum { full,
       semi,
       burst, };
unsigned char autostate = full;


void setup() {
  // initialize the LED pin as an output:
  pinMode(muzzlePin, OUTPUT);
  // initialize the trigger and safety pins as input:
  pinMode(triggerPin, INPUT);
  pinMode(safetyPin, INPUT);
  // initialize the audio pins
  pinMode(startSoundPin, INPUT);
  pinMode(clickSoundPin, INPUT);
//  pinMode(fireTailSoundPin, INPUT);
  pinMode(fireSoundPin, INPUT);
  pinMode(grenadeSoundPin, INPUT);
//  pinMode(triggerPin, INPUT); //duplicate
//  pinMode(safetyPin, INPUT);  // duplicate
  // These are the additional pin declarations.
  pinMode(grenadePin, INPUT);
  pinMode(fullPin, INPUT);
  pinMode(semiPin, INPUT);

  // set up the audio trigger pins to give a path to GND when set to OUTPUT
  // this sets the initial led status to off on the muzzle strobe LEDs
  digitalWrite(muzzlePin, LOW);

  digitalWrite(triggerPin, HIGH);
  digitalWrite(safetyPin, HIGH);
  // These are the additional button pin declarations.
  digitalWrite(grenadePin, HIGH);
  digitalWrite(fullPin, HIGH);
  digitalWrite(semiPin, HIGH);

  Serial.begin(9600);
  Serial1.begin(9600);
  ss.begin(9600);
  alpha4.begin(0x70);  // pass in the address
  alpha5.begin(0x71);

  delay(200);  //give the audio board time to power up; otherwise bootup sound will be called before audio board is ready

  Serial1.print("#2\n"); //this plays the bootup sound
  // Change this number if you change the default ammo count above to match
  alpha4.writeDigitAscii(0, '0');
  alpha4.writeDigitAscii(1, '2');
  alpha4.writeDigitAscii(2, '9');
  alpha4.writeDigitAscii(3, '9');
  alpha4.writeDisplay();
  alpha5.writeDigitAscii(0, '0');
  alpha5.writeDigitAscii(1, '1');
  alpha5.writeDigitAscii(2, '9');
  alpha5.writeDigitAscii(3, '9');
  alpha5.writeDisplay();
  delay(100);
  // these are the round load effects
  for (int x = 4; x > 0; x--) {
    updateAmmoCounter();
    ammoCount--;
    delay(50);         
  }
  updateAmmoCounter();
 
  for (int x = 1; x > 0; x--) {
    updateGrenadeCounter();
    grenadeCount--;
    delay(50);         
  }
  updateGrenadeCounter();   
}
// this function calls the current value of ammoCount and writes it to the LED display
void updateAmmoCounter() {
  String ammoDisp = String(ammoCount);
  if (ammoCount < 10) {
            alpha4.writeDigitAscii(0, '0');
            alpha4.writeDigitAscii(1, '0');
            alpha4.writeDigitAscii(2, '0');
            alpha4.writeDigitAscii(3, ammoDisp[0]);
          }
          else if (ammoCount < 100) {
            alpha4.writeDigitAscii(0, '0');
            alpha4.writeDigitAscii(1, '0');
            alpha4.writeDigitAscii(2, ammoDisp[0]);
            alpha4.writeDigitAscii(3, ammoDisp[1]);
          }
          else if (ammoCount < 1000) {
            alpha4.writeDigitAscii(0, '0');
            alpha4.writeDigitAscii(1, ammoDisp[0]);
            alpha4.writeDigitAscii(2, ammoDisp[1]);
            alpha4.writeDigitAscii(3, ammoDisp[2]);
          }
          else if (ammoCount < 10000) {
            alpha4.writeDigitAscii(0, ammoDisp[0]);
            alpha4.writeDigitAscii(1, ammoDisp[1]);
            alpha4.writeDigitAscii(2, ammoDisp[2]);
            alpha4.writeDigitAscii(3, ammoDisp[3]);
          }
          alpha4.writeDisplay();
}
// this displays and writes the grenade ammo count
void updateGrenadeCounter() {
  String grenadeDisp = String(grenadeCount);
  if (grenadeCount < 10) {
            alpha5.writeDigitAscii(0, '0');
            alpha5.writeDigitAscii(1, '0');
            alpha5.writeDigitAscii(2, '0');
            alpha5.writeDigitAscii(3, grenadeDisp[0]);
          }
          else if (grenadeCount < 100) {
            alpha5.writeDigitAscii(0, '0');
            alpha5.writeDigitAscii(1, '0');
            alpha5.writeDigitAscii(2, grenadeDisp[0]);
            alpha5.writeDigitAscii(3, grenadeDisp[1]);
          }
          else if (grenadeCount < 1000) {
            alpha5.writeDigitAscii(0, '0');
            alpha5.writeDigitAscii(1, grenadeDisp[0]);
            alpha5.writeDigitAscii(2, grenadeDisp[1]);
            alpha5.writeDigitAscii(3, grenadeDisp[2]);
          }
          else if (grenadeCount < 10000) {
            alpha5.writeDigitAscii(0, grenadeDisp[0]);
            alpha5.writeDigitAscii(1, grenadeDisp[1]);
            alpha5.writeDigitAscii(2, grenadeDisp[2]);
            alpha5.writeDigitAscii(3, grenadeDisp[3]);
          }
          alpha5.writeDisplay();
}
void setSafe() {
  safetyOn = true;
  alpha4.clear();
  alpha4.writeDisplay();
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'S');
  alpha4.writeDigitAscii(1, 'A');
  alpha4.writeDigitAscii(2, 'F');
  alpha4.writeDigitAscii(3, 'E');
  alpha4.writeDisplay();
  alpha5.writeDigitAscii(0, 'S');
  alpha5.writeDigitAscii(1, 'A');
  alpha5.writeDigitAscii(2, 'F');
  alpha5.writeDigitAscii(3, 'E');
  alpha5.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'S');
  alpha4.writeDigitAscii(1, 'A');
  alpha4.writeDigitAscii(2, 'F');
  alpha4.writeDigitAscii(3, 'E');
  alpha4.writeDisplay();
  alpha5.writeDigitAscii(0, 'S');
  alpha5.writeDigitAscii(1, 'A');
  alpha5.writeDigitAscii(2, 'F');
  alpha5.writeDigitAscii(3, 'E');
  alpha5.writeDisplay();
}

void setArm() {
  safetyOn = false;
  alpha4.writeDigitAscii(0, ' ');
  alpha4.writeDigitAscii(1, 'A');
  alpha4.writeDigitAscii(2, 'R');
  alpha4.writeDigitAscii(3, 'M');
  alpha4.writeDisplay();
  alpha5.writeDigitAscii(0, ' ');
  alpha5.writeDigitAscii(1, 'A');
  alpha5.writeDigitAscii(2, 'R');
  alpha5.writeDigitAscii(3, 'M');
  alpha5.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, ' ');
  alpha4.writeDigitAscii(1, 'A');
  alpha4.writeDigitAscii(2, 'R');
  alpha4.writeDigitAscii(3, 'M');
  alpha4.writeDisplay();
  alpha5.writeDigitAscii(0, ' ');
  alpha5.writeDigitAscii(1, 'A');
  alpha5.writeDigitAscii(2, 'R');
  alpha5.writeDigitAscii(3, 'M');
  alpha5.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, ' ');
  alpha4.writeDigitAscii(1, 'A');
  alpha4.writeDigitAscii(2, 'R');
  alpha4.writeDigitAscii(3, 'M');
  alpha4.writeDisplay();
  alpha5.writeDigitAscii(0, ' ');
  alpha5.writeDigitAscii(1, 'A');
  alpha5.writeDigitAscii(2, 'R');
  alpha5.writeDigitAscii(3, 'M');
  alpha5.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, ' ');
  alpha4.writeDigitAscii(1, 'A');
  alpha4.writeDigitAscii(2, 'R');
  alpha4.writeDigitAscii(3, 'M');
  alpha4.writeDisplay();
  alpha5.writeDigitAscii(0, ' ');
  alpha5.writeDigitAscii(1, 'A');
  alpha5.writeDigitAscii(2, 'R');
  alpha5.writeDigitAscii(3, 'M');
  alpha5.writeDisplay();
  delay(300);
  updateAmmoCounter();
  updateGrenadeCounter();
}








void setSemi() {
  semiOn = false;
  alpha4.writeDigitAscii(0, 'S');
  alpha4.writeDigitAscii(1, 'E');
  alpha4.writeDigitAscii(2, 'M');
  alpha4.writeDigitAscii(3, 'I');
  alpha4.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'S');
  alpha4.writeDigitAscii(1, 'E');
  alpha4.writeDigitAscii(2, 'M');
  alpha4.writeDigitAscii(3, 'I');
  alpha4.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'S');
  alpha4.writeDigitAscii(1, 'E');
  alpha4.writeDigitAscii(2, 'M');
  alpha4.writeDigitAscii(3, 'I');
  alpha4.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'S');
  alpha4.writeDigitAscii(1, 'E');
  alpha4.writeDigitAscii(2, 'M');
  alpha4.writeDigitAscii(3, 'I');
  alpha4.writeDisplay();
  delay(30);
  updateAmmoCounter();
}


void setSemig() {
  semiOn = false;
  alpha5.writeDigitAscii(0, 'S');
  alpha5.writeDigitAscii(1, 'E');
  alpha5.writeDigitAscii(2, 'M');
  alpha5.writeDigitAscii(3, 'I');
  alpha5.writeDisplay();
  delay(50);
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha5.writeDigitAscii(0, 'S');
  alpha5.writeDigitAscii(1, 'E');
  alpha5.writeDigitAscii(2, 'M');
  alpha5.writeDigitAscii(3, 'I');
  alpha5.writeDisplay();
  delay(50);
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha5.writeDigitAscii(0, 'S');
  alpha5.writeDigitAscii(1, 'E');
  alpha5.writeDigitAscii(2, 'M');
  alpha5.writeDigitAscii(3, 'I');
  alpha5.writeDisplay();
  delay(50);
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha5.writeDigitAscii(0, 'S');
  alpha5.writeDigitAscii(1, 'E');
  alpha5.writeDigitAscii(2, 'M');
  alpha5.writeDigitAscii(3, 'I');
  alpha5.writeDisplay();
  delay(30);
  updateGrenadeCounter();
}

void setFull() {
  semiOn = false;
  alpha4.writeDigitAscii(0, 'F');
  alpha4.writeDigitAscii(1, 'U');
  alpha4.writeDigitAscii(2, 'L');
  alpha4.writeDigitAscii(3, 'L');
  alpha4.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'F');
  alpha4.writeDigitAscii(1, 'U');
  alpha4.writeDigitAscii(2, 'L');
  alpha4.writeDigitAscii(3, 'L');
  alpha4.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'F');
  alpha4.writeDigitAscii(1, 'U');
  alpha4.writeDigitAscii(2, 'L');
  alpha4.writeDigitAscii(3, 'L');
  alpha4.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'F');
  alpha4.writeDigitAscii(1, 'U');
  alpha4.writeDigitAscii(2, 'L');
  alpha4.writeDigitAscii(3, 'L');
  alpha4.writeDisplay();
  delay(30);
  updateAmmoCounter();
}
void setBrst() {
  burstOn = true;
  alpha4.writeDigitAscii(0, 'B');
  alpha4.writeDigitAscii(1, 'R');
  alpha4.writeDigitAscii(2, 'S');
  alpha4.writeDigitAscii(3, 'T');
  alpha4.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'B');
  alpha4.writeDigitAscii(1, 'R');
  alpha4.writeDigitAscii(2, 'S');
  alpha4.writeDigitAscii(3, 'T');
  alpha4.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'B');
  alpha4.writeDigitAscii(1, 'R');
  alpha4.writeDigitAscii(2, 'S');
  alpha4.writeDigitAscii(3, 'T');
  alpha4.writeDisplay();
  delay(50);
  alpha4.clear();
  alpha4.writeDisplay();
  delay(50);
  alpha4.writeDigitAscii(0, 'B');
  alpha4.writeDigitAscii(1, 'R');
  alpha4.writeDigitAscii(2, 'S');
  alpha4.writeDigitAscii(3, 'T');
  alpha4.writeDisplay();
  delay(30);
  updateAmmoCounter();
}

void setBrstg() {
  burstOn = true;
  alpha5.writeDigitAscii(0, 'B');
  alpha5.writeDigitAscii(1, 'R');
  alpha5.writeDigitAscii(2, 'S');
  alpha5.writeDigitAscii(3, 'T');
  alpha5.writeDisplay();
  delay(50);
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha5.writeDigitAscii(0, 'B');
  alpha5.writeDigitAscii(1, 'R');
  alpha5.writeDigitAscii(2, 'S');
  alpha5.writeDigitAscii(3, 'T');
  alpha5.writeDisplay();
  delay(50);
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha5.writeDigitAscii(0, 'B');
  alpha5.writeDigitAscii(1, 'R');
  alpha5.writeDigitAscii(2, 'S');
  alpha5.writeDigitAscii(3, 'T');
  alpha5.writeDisplay();
  delay(50);
  alpha5.clear();
  alpha5.writeDisplay();
  delay(50);
  alpha5.writeDigitAscii(0, 'B');
  alpha5.writeDigitAscii(1, 'R');
  alpha5.writeDigitAscii(2, 'S');
  alpha5.writeDigitAscii(3, 'T');
  alpha5.writeDisplay();
  delay(30);
  updateGrenadeCounter();
}


// --MAIN LOOP STARTS HERE--
void loop() {
 
// begin safety pin and firemode selection section
  safetyState = digitalRead(safetyPin);
  if (safetyState != lastSafetyState) { if (safetyState == LOW) { if (safetyOn == false) { setSafe(); } else if (safetyOn == true) { setArm(); } } }
  semiState = digitalRead(semiPin);
  if (semiState != lastsemiState) { if (semiState == LOW) { if (semiOn == true) {  } else if (semiOn == false) { fullbuttonCount++; } }}
  fullState = digitalRead(fullPin);   
  if (fullState != lastfullState) { if (fullState == LOW) { if (fullOn == true) {  } else if (fullOn == false) { semibuttonCount++; } } }
  if (semibuttonCount == 1){setBrstg();semibuttonCount++;}
  if (semibuttonCount == 3){setSemig();semibuttonCount++;}
//  if (semibuttonCount == 5){setFull();semibuttonCount++;}
  if (semibuttonCount == 5){semibuttonCount=0;}
  if (fullbuttonCount == 1){setBrst();fullbuttonCount++;}   
  if (fullbuttonCount == 3){setSemi();fullbuttonCount++;}
  if (fullbuttonCount == 5){setFull();fullbuttonCount++;}
  if (fullbuttonCount == 7){fullbuttonCount=0;}

// end safety pin and firemode selection section

// grenade trigger section
  if (semibuttonCount ==2) { //brst
  grenadetriggerState = digitalRead(grenadePin);
  if (grenadetriggerState != lastgrenadetriggerState){
    if (grenadetriggerState == HIGH) { } // this can be used to add a grenade flash using a declared pin later. the ethernet cable has no spare pins currently.  It would be something along these lines: digitalWrite(muzzlePin, LOW);
    if (grenadetriggerState == LOW && safetyOn == true) { Serial1.print("#1\n"); grenadetriggerState = digitalRead(grenadePin);  }
    if (grenadeCount <= 0) { grenadetriggerState = digitalRead(grenadePin); }
    if (safetyOn == false && grenadeCount > 0) { Serial1.print("#0\n");   grenadeCount--; updateGrenadeCounter(); delay(300); grenadetriggerState = digitalRead(grenadePin);}
    if (safetyOn == false && grenadeCount == 0) { Serial1.print("q\n"); Serial1.print("#1\n"); }     
    if (safetyOn == false && grenadetriggerState == HIGH) {  Serial1.print("q\n"); }   
    lastgrenadetriggerState = grenadetriggerState;
    }
  }
  if (semibuttonCount ==4 || semibuttonCount == 0) { // semi
    grenadetriggerState = digitalRead(grenadePin);
    if (grenadetriggerState != lastgrenadetriggerState){
      if (grenadetriggerState == HIGH) { } // this can be used to add a grenade flash using a declared pin later. the ethernet cable has no spare pins currently.  It would be something along these lines: digitalWrite(muzzlePin, LOW);
      if (grenadetriggerState == LOW && safetyOn == true) { Serial1.print("#1\n"); grenadetriggerState = digitalRead(grenadePin);  }
      if (grenadeCount <= 0) { grenadetriggerState = digitalRead(grenadePin); }
      if (safetyOn == false && grenadeCount > 0) { Serial1.print("#0\n");   grenadeCount--; updateGrenadeCounter(); delay(600); grenadetriggerState = digitalRead(grenadePin);}
      if (safetyOn == false && grenadeCount == 0) { Serial1.print("q\n"); Serial1.print("#1\n"); }     
      if (safetyOn == false && grenadetriggerState == HIGH) {  Serial1.print("q\n"); }   
      lastgrenadetriggerState = grenadetriggerState;
      }
  }
 
// end grenade trigger section
//begin fireselect mode section
  if (fullbuttonCount == 2  ){
    triggerState = digitalRead(triggerPin);     
    if (triggerState != lastTriggerState) {
      if (triggerState == HIGH) { digitalWrite(muzzlePin, LOW); }       
      if (triggerState == LOW && safetyOn == true) { Serial1.print("#1\n"); triggerState = digitalRead(triggerPin);  }
      if (ammoCount <= 0) { triggerState = digitalRead(triggerPin); }
      if (safetyOn == false && ammoCount > 0) { Serial1.print("#3\n");  digitalWrite(muzzlePin, HIGH); ammoCount--; updateAmmoCounter(); delay(10); digitalWrite(muzzlePin, LOW); ammoCount--; updateAmmoCounter(); delay(20); triggerState = digitalRead(triggerPin); }
      if (safetyOn == false && ammoCount == 0 ){ digitalWrite(muzzlePin, LOW); Serial1.print("q\n"); delay(10); Serial1.print("#1\n"); }
      if (safetyOn == false && triggerState == HIGH) { digitalWrite(muzzlePin, LOW); Serial.print("q\n"); }
      } lastTriggerState = triggerState;
  }   
  if (fullbuttonCount == 4){
    triggerState = digitalRead(triggerPin);     
    if (triggerState != lastTriggerState) {
      if (triggerState == HIGH) { digitalWrite(muzzlePin, LOW); }       
      if (triggerState == LOW && safetyOn == true) { Serial1.print("#1\n"); triggerState = digitalRead(triggerPin);  }
      if (ammoCount <= 0) { triggerState = digitalRead(triggerPin); }
      if (safetyOn == false && ammoCount > 0) { Serial1.print("#3\n");  digitalWrite(muzzlePin, HIGH); ammoCount--; updateAmmoCounter(); delay(10); digitalWrite(muzzlePin, LOW); delay(509); triggerState = digitalRead(triggerPin); }
      if (safetyOn == false && ammoCount == 0 ){ digitalWrite(muzzlePin, LOW); Serial1.print("q\n"); delay(10); Serial1.print("#1\n"); }
      if (safetyOn == false && triggerState == HIGH) { digitalWrite(muzzlePin, LOW); Serial.print("q\n"); }
    } lastTriggerState = triggerState;
  }
  if (fullbuttonCount == 6|| fullbuttonCount == 0){
    triggerState = digitalRead(triggerPin);     
    if (triggerState != lastTriggerState) {
    if (triggerState == HIGH) { digitalWrite(muzzlePin, LOW); }       
    while (triggerState == LOW) { if (safetyOn == true) { Serial1.print("#1\n"); triggerState = digitalRead(triggerPin); return; }
    if (ammoCount <= 0) { triggerState = digitalRead(triggerPin); return; }
    if (safetyOn == false) {
    if (ammoCount > 0) { Serial1.print("#3\n");  digitalWrite(muzzlePin, HIGH); ammoCount--;
    updateAmmoCounter(); delay(10); digitalWrite(muzzlePin, LOW); delay(39); triggerState = digitalRead(triggerPin);
    if (triggerState == HIGH) { digitalWrite(muzzlePin, LOW); Serial.print("q\n"); }
    if (ammoCount == 0){ digitalWrite(muzzlePin, LOW); Serial1.print("q\n"); delay(10); Serial1.print("#1\n"); }
    } } else { return; } } lastTriggerState = triggerState;}
  }
  lastSafetyState = safetyState;   
  lastsemiState = semiState;   
  lastfullState = fullState;
// end mg fire mode selection section
}     

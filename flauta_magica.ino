/*********************************************************
  Flauta Magica
**********************************************************/
#include "M16.h"
#include "Osc.h"
#include "Env.h"
#include "MultiControl.h"

#define NUM_TOUCH_PADS 6 


//** Setup touch for multicontrol
MultiControl touchPin[NUM_TOUCH_PADS] = {
  MultiControl (1, 0),
  MultiControl (2, 0),
  MultiControl (3, 0),
  MultiControl (4, 0),
  MultiControl (5, 0),
  MultiControl (6, 0)
};

//** Setup Pot pin for multicontrol 
MultiControl volPotPin13(13, 1); // set pin 13 for volume potentiometer

//** Flags for turning off button
bool flag[NUM_TOUCH_PADS] = {0, 0, 0, 0, 0, 0};
// int scale [] = {0, 2, 4, 5, 7, 9, 0 };

const int touchThreshold = 50; //Umbral del Sensor

//** Audio Stuff
int16_t waveTable[TABLE_SIZE]; // empty wavetable
Osc aOsc1(waveTable);
int16_t volPot = 250; 
unsigned long msNow, envTime, pitchTime;
Env ampEnv1;


void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("ESP32 - Flauta Magica");
  Osc::sinGen(waveTable);
  // seti2sPins(16, 4, 21, 12); // Custom pins I2S
  ampEnv1.setAttack(10);
  ampEnv1.setSustain(0.5);
  ampEnv1.setRelease(200);
  audioStart();
}

//** Control Stuff
//Function to handle touch
void handleTouch(int i) {
  int touchRaw = touchPin[i].readTouch();
  if (touchRaw > touchThreshold) {
      // Serial.println(touchRaw);
    while (flag[i] == 0) {
      Serial.println("on p" + String(i + 4));
      int pitch = 48 + i + random(8);
      aOsc1.setPitch(pitch);
      // int pitch = pitchQuantize(48 + i, scale, 0);
      // aOsc1.setPitch(pitch);
      Serial.println("Pitch " + String(pitch));
      ampEnv1.start();
      flag[i] = 1;
    }  
  } else {
    if (flag[i] == 1) {
      Serial.println("off a" + String(i + 4));
      ampEnv1.startRelease();
      flag[i] = 0;
    }
  }
}


void loop() {
  //** Control 
  // touch handler
  for (int i=0; i<NUM_TOUCH_PADS; i++) {
    handleTouch(i);
  }
  // Vol Pot
  int16_t preVolPot = volPot;
  int16_t newVolPot = volPotPin13.readPot();
  if( newVolPot != preVolPot) {
    volPot = newVolPot;
    Serial.println("Vol pot: " + String(volPot));
  }
  //** M16 audio 
  msNow = millis();
  
  if (msNow > pitchTime) {
    pitchTime = msNow + 1000;
  } // end Pitch time loop
  
  if (msNow > envTime){
    envTime = msNow + 1;
    ampEnv1.next();
  }
}

/* The audioUpdate function is required in all M16 programs 
* to specify the audio sample values to be played.
* Always finish with i2s_write_samples()
*/
void audioUpdate() {
  int16_t oscVal = (aOsc1.next() * ampEnv1.getValue())>>16;
  int16_t leftVal = (oscVal * volPot)>>10;
  int16_t rightVal = leftVal;
  i2s_write_samples(leftVal, rightVal);
}


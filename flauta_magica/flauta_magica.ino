/*********************************************************
  Flauta Magica
**********************************************************/
#include "M16.h"
#include "Osc.h"
#include "Env.h"
#include "MultiControl.h"

// Num of Touch pads for Multicontrol
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
bool touchFlags[NUM_TOUCH_PADS] = {0,0,0,0,0,0};

//** Audio Components
int16_t waveTables[NUM_TOUCH_PADS][TABLE_SIZE]; // Wavetable for Oscilator
Osc aOsci[NUM_TOUCH_PADS] = {
  Osc(waveTables[0]),
  Osc(waveTables[1]),
  Osc(waveTables[2]),
  Osc(waveTables[3]),
  Osc(waveTables[4]),
  Osc(waveTables[5])
};

Env ampEnv[NUM_TOUCH_PADS];

// Const and Global Variables
const int touchThreshold = 10; //Umbral del Sensor
const int pitches[] = {69, 72, 74, 76, 78, 80}; // Base pitches for each touch pin
int16_t volPot = 250;  // Volume potentiometer
unsigned long msNow, envTime, pitchTime;

//** Init Osc and Envs
void initAudioComponents() {
  for(int i = 0; i < NUM_TOUCH_PADS; ++i){
    Osc::sawGen(waveTables[i]);
    ampEnv[i].setAttack(10);
    ampEnv[i].setSustain(0.5);
    ampEnv[i].setRelease(200);
  }
}

//** Function to process touch input
void handleTouch(int i, MultiControl &control, Osc &osc, Env &env, int pitch) {
  int touchRaw = touchPin[i].readTouch();
  if (touchRaw > touchThreshold) {
    // Serial.println(touchRaw);
    while (touchFlags[i] == 0) {
      Serial.println("Touch detected on pin " + String(i));
      // int pitch = pitchQuantize(48 + i, scale, 0);
      int pitch = pitch + random(4);
      Serial.println("Pitch " + String(pitch));
      osc.setPitch(pitch);
      env.start();
      touchFlags[i] = 1;
    }  
  } else {
    if (touchFlags[i] == 1) {
      Serial.println("Touch released on pin " + String(i));
      env.startRelease();
      touchFlags[i] = 0;
    }
  }
}


void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("ESP32 - Flauta Magica");
  // seti2sPins(16, 4, 21, 12); // Custom pins I2S
  initAudioComponents();
  audioStart();
}


void loop() {
  // Touch Handler 
  for (int i = 0; i < NUM_TOUCH_PADS; ++i) {
    handleTouch(i, touchPin[i], aOsci[i], ampEnv[i], pitches[i]);
  }

  //** M16 audio s
  msNow = millis();
  
  // Pitch loop
  if (msNow > pitchTime) {
    pitchTime = msNow + 1000;
  } 

  // Env loop
  if (msNow > envTime){
    envTime = msNow + 1;
    for (int i = 0; i < NUM_TOUCH_PADS; ++i){
      ampEnv[i].next();
    }
  } 

    // Vol Pot
  // int16_t preVolPot = volPot;
  // int16_t newVolPot = volPotPin13.readPot();
  // if( newVolPot != preVolPot) {
  //   volPot = newVolPot;
  //   Serial.println("Vol pot: " + String(volPot));
  // }
}

/* The audioUpdate function is required in all M16 programs 
* to specify the audio sample values to be played.
* Always finish with i2s_write_samples()
*/
void audioUpdate() {
  // int16_t oscVal = (aOsc1.next() * ampEnv1.getValue())>>16;
  int16_t oscVal = 0;

  // Sum oscillator outputs
  for (int i = 0; i < NUM_TOUCH_PADS; ++i) {
    oscVal += (aOsci[i].next() * ampEnv[i].getValue()) >> 16;
  }

  int16_t leftVal = (oscVal * volPot)>>10;
  int16_t rightVal = leftVal;
  i2s_write_samples(leftVal, rightVal);
}


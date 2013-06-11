#include <EngduinoLEDs.h>
#include <EngduinoButton.h>
#include <EngduinoAccelerometer.h>
#include <EngduinoIR.h>
#include <EngduinoThermistor.h>
#include <Wire.h>

// This is a fusion of code for a selection of the Engduino example
// programs - allowing simple demonstration of some of the Engduino's
// capabilities and an illustration of some of the simple products that
// might be formed using the Engduino.
//
// When this starts up, there is a running light on the LHS of the
// Engduino and one pushes the button a number of times both to light
// LEDS on the RHS of the Engduino and to select the appropriate demo.
// The demo is selected after the button has not be pressed for 3
// seconds. Once selected, the chosen demo will run until the device
// is reset.
//
// 1: runningLEDs
// 2: level
// 3: temperatureLEDs
// 4: comms
// 5: fizzBuzz
// 6: reactionGame
// 7: pulseSensor
// 8: raspberryPi_simple
// 9: moodLighting
//


// State holds which, if any, demo is selected and when
// that selection was made.
//
int  selected = 0;
long selectionTime;

// N_DEMOS is the number of demos we have - and so the max number of LEDs we light on the RHS when choosing
// which to run.
// 
#define N_DEMOS 9

// The colours we wish to use for the LEDs
//
colour colours[16] = {RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN, WHITE, RED, GREEN};

// This is state for the LEDs that run up and down the LHS of the Engduino
// until a choice is made.
//
int  kitLED = 10;
int  kitDir = 1;
long kitLast;

void setup()
{
  // Run the serial fast
  Serial.begin(115200);
  
  // And start up all the Engduino devices
  //
  EngduinoLEDs.begin();
  EngduinoButton.begin();  
  EngduinoThermistor.begin();
  EngduinoAccelerometer.begin();
  EngduinoIR.begin();
  
  randomSeed(analogRead(0));    // Seed the random number generator from analog input 0, which is unconnected
  
  // Set the first of the running LEDs
  EngduinoLEDs.setLED(kitLED, colours[kitLED-1]);
  kitLast = millis();
}

void loop() {
  // If the button was pressed since we last looked, change the selected demo
  // and mark the time of this change
  //
   if (EngduinoButton.wasPressed()) {
      if (selected == N_DEMOS) {
        selected = 1;
        EngduinoLEDs.setAll(OFF);
      }
      else
        selected++;
      selectionTime = millis();
  }
 
  // set the LEDs based on the selection
  //
  if (selected > 0) {
    EngduinoLEDs.setLED(selected, colours[selected-1]);
  
    // Check to see if the time since selection is longer 
    // than 3s, and if it is, fix that selection and start
    // running the appropriate demo
    //
    if (millis() - selectionTime > 3000) {
      EngduinoLEDs.setAll(OFF);
      switch (selected) {
        case 1:
          runningLEDs();
        case 2:
          level();
        case 3:
          temperatureLEDs();            
        case 4:
          comms();
        case 5:
          fizzBuzz();
        case 6:
          reactionGame();
        case 7:
          pulseSensor();
        case 8:
          raspberryPi_simple();
        case 9:
          moodLighting();
      }
    }
  }
  
  // Change the running LED displayed if it's been
  // displayed for > 200ms.
  // 
  if (millis() - kitLast > 200) {
    // Switch the old LED off, choose the new LED, and switch that on
    //
    EngduinoLEDs.setLED(kitLED, OFF);    
    kitLED += kitDir;
    EngduinoLEDs.setLED(kitLED, colours[kitLED-1]);
    
    // Do we need to reverse direction?
    //
    if (kitLED <= 10 || kitLED >= 14)
      kitDir = -kitDir;
    
    // Record when we last made a change
    //
    kitLast = millis();
  }
}

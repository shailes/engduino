// This is a sophisticated example of interfacing to the Engduino.
//
// It makes use of a pulse sensor that can be obtained from www.pulsesensor.com
// This returns an analogue signal, which is picked up on the IO_1 expansion pin
// and the code does some processing to determine which part of a heartbeat the
// signal belongs to. It outputs this in the form of flashing LEDs and, in more
// detail over the serial link. This can be shown on a GUI, an example of which
// is included as 'Processing' code. To run that code you will need to download
// Processing from http://processing.org/ Clearly it would also be possible to
// write a similar interface in Python, Java, or the web technology of your choice
//


//  VARIABLES
int pulsePin = 7;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

volatile int rate[10];                    // used to hold last ten IBI values
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find the inter beat interval
volatile int P =512;                      // used to find peak in pulse wave
volatile int T = 512;                     // used to find trough in pulse wave
volatile int thresh = 512;                // used to find instant moment of heart beat
volatile int amp = 100;                   // used to hold amplitude of pulse waveform
volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = true;       // used to seed rate array so we startup with reasonable BPM
 
void pulseSensor() {
  /*
  >> Pulse Sensor Amped 1.1 <<
  This code is for Pulse Sensor Amped by Joel Murphy and Yury Gitman
      www.pulsesensor.com 
      >>> Pulse Sensor purple wire goes to Analog Pin 0 <<<
  Pulse Sensor sample aquisition and processing happens in the background via Timer 2 interrupt. 2mS sample rate.
  PWM on pins 3 and 11 will not work when using this code, because we are using Timer 2!
  The following variables are automatically updated:
  Signal :    int that holds the analog signal data straight from the sensor. updated every 2mS.
  IBI  :      int that holds the time interval between beats. 2mS resolution.
  BPM  :      int that holds the heart rate value, derived every beat, from averaging previous 10 IBI values.
  QS  :       boolean that is made true whenever Pulse is found and BPM is updated. User must reset.
  Pulse :     boolean that is true when a heartbeat is sensed then false in time with pin13 LED going out.
  
  This code is designed with output serial data to Processing sketch "PulseSensorAmped_Processing-xx"
  The Processing sketch is a simple data visualizer. 
  All the work to find the heartbeat and determine the heartrate happens in the code below.
  Pin 13 LED will blink with heartbeat.
  If you want to use pin 13 for something else, adjust the interrupt handler
  It will also fade the Engduino LEDs with every beat.
  Check here for detailed code walkthrough - but note that the code here has been very slighly modified for Engduino:
  http://pulsesensor.myshopify.com/pages/pulse-sensor-amped-arduino-v1dot1
  
  Code Version 02 by Joel Murphy & Yury Gitman  Fall 2012
  This update changes the HRV variable name to IBI, which stands for Inter-Beat Interval, for clarity.
  Switched the interrupt to Timer2.  500Hz sample rate, 2mS resolution IBI value.
  Fade LED pin moved to pin 5 (use of Timer2 disables PWM on pins 3 & 11).
  Tidied up inefficiencies since the last version. 
  */


  // Initializes Timer1 to throw an interrupt every 2ms.
  // With prescaler at /64, each tick is 8us
  //
  TCCR1A = 0x00;    // GO INTO CTC MODE
  TCCR1B = 0x0B;    // DON'T FORCE COMPARE, /64 PRESCALER 
  OCR1AH = 0X00;    // SET THE MAX COUNT TO 249 FOR 500Hz SAMPLE RATE
  OCR1AL = 0XF9;    // SET THE MAX COUNT TO 249 FOR 500Hz SAMPLE RATE
  TCNT1H = 0x00;    // SET THE COUNTER SO WE START FROM ZERO
  TCNT1L = 0x00;    // SET THE COUNTER SO WE START FROM ZERO
  TIMSK1 = 0x02;    // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A
  sei();            // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED      
     // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
     // AND APPLY THAT VOLTAGE TO THE A-REF PIN
  analogReference(EXTERNAL);   
  

  while (true) {
    sendDataToProcessing('S', Signal);     // send Processing the raw Pulse Sensor data
    if (QS == true){                       // Quantified Self flag is true when arduino finds a heartbeat
          fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
          sendDataToProcessing('B',BPM);   // send heart rate with a 'B' prefix
          sendDataToProcessing('Q',IBI);   // send time between beats with a 'Q' prefix
          QS = false;                      // reset the Quantified Self flag for next time    
       }

    ledFadeToBeat();
    
    delay(20);                             //  take a break
  }
}

void ledFadeToBeat(){
    fadeRate -= 15;                         //  set LED fade value
    fadeRate = constrain(fadeRate,0,255);   //  keep LED fade value from going into negative numbers!
    EngduinoLEDs.setAll(RED, fadeRate);
  }

void sendDataToProcessing(char symbol, int data )
{
    Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
    Serial.println(data);                // the data to send culminating in a carriage return
}

// THIS IS THE TIMER 2 INTERRUPT SERVICE ROUTINE. 
// Timer 2 makes sure that we take a reading every 2 miliseconds
ISR(TIMER1_COMPA_vect){                         // triggered when Timer2 counts to 124
    cli();                                      // disable interrupts while we do this
    Signal = analogRead(pulsePin);              // read the Pulse Sensor 
    sampleCounter += 2;                         // keep track of the time in mS with this variable
    int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

    //  find the peak and trough of the pulse wave
    if(Signal < thresh && N > (IBI/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal < T){                        // T is the trough
            T = Signal;                         // keep track of lowest point in pulse wave 
         }
       }
      
    if(Signal > thresh && Signal > P){          // thresh condition helps avoid noise
        P = Signal;                             // P is the peak
       }                                        // keep track of highest point in pulse wave
    
  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N > 250){                                 // avoid high frequency noise
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){        
      Pulse = true;                             // set the Pulse flag when we think there is a pulse
      digitalWrite(blinkPin,HIGH);              // turn on pin 13 LED
      IBI = sampleCounter - lastBeatTime;       // measure time between beats in mS
      lastBeatTime = sampleCounter;             // keep track of time for next pulse
           
           if(firstBeat){                       // if it's the first time we found a beat, if firstBeat == TRUE
               firstBeat = false;               // clear firstBeat flag
               return;                          // IBI value is unreliable so discard it
              }   
           if(secondBeat){                      // if this is the second beat, if secondBeat == TRUE
              secondBeat = false;               // clear secondBeat flag
                 for(int i=0; i<=9; i++){       // seed the running total to get a realisitic BPM at startup
                      rate[i] = IBI;                      
                      }
              }
            
      // keep a running total of the last 10 IBI values
      word runningTotal = 0;                    // clear the runningTotal variable    
  
      for(int i=0; i<=8; i++){                  // shift data in the rate array
            rate[i] = rate[i+1];                // and drop the oldest IBI value 
            runningTotal += rate[i];            // add up the 9 oldest IBI values
          }
          
      rate[9] = IBI;                            // add the latest IBI to the rate array
      runningTotal += rate[9];                  // add the latest IBI to runningTotal
      runningTotal /= 10;                       // average the last 10 IBI values 
      BPM = 60000/runningTotal;                 // how many beats can fit into a minute? that's BPM!
      QS = true;                                // set Quantified Self flag 
      // QS FLAG IS NOT CLEARED INSIDE THIS ISR
      }                       
  }

  if (Signal < thresh && Pulse == true){     // when the values are going down, the beat is over
      digitalWrite(blinkPin,LOW);            // turn off pin 13 LED
      Pulse = false;                         // reset the Pulse flag so we can do it again
      amp = P - T;                           // get amplitude of the pulse wave
      thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
      P = thresh;                            // reset these for next time
      T = thresh;
     }
  
  if (N > 2500){                             // if 2.5 seconds go by without a beat
      thresh = 512;                          // set thresh default
      P = 512;                               // set P default
      T = 512;                               // set T default
      lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date        
      firstBeat = true;                      // set these to avoid noise
      secondBeat = true;                     // when we get the heartbeat back
     }
  
  sei();                                     // enable interrupts when you're done!
}// end isr




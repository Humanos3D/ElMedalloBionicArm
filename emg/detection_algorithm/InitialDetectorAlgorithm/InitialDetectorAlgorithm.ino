/*
  InitialDetectorAlgorithm

  Development code for figuring out how to provide a robust signal
  from the Protesis Avanzada EMG sensor

  Bryn Davis

*/

#include <Filters.h>   // From https://github.com/JonHub/Filters

#define SensorInputPin A5 // input pin number

// Fixed parameters
int sensor = 1;                   // 0 for Protesis Avanzada; 1 for OYMotion
float filterFrequency = 0.2;      // Change rate to be considered background (Hz)
float fall_time = 1000;           // Signal must be low this long for 1 -> 0 (ms)

// Sensor specific parameters
// if (sensor == 0){
// float threshold = .5;             // Voltage above background to register signal
// float rise_time = 100;            // Must see signal this long for 0 -> 1 (ms)
// float background_timeout = 5000;  // Max time to not calculate background (ms)
// }
// else if (sensor == 1){
float threshold = .25;             // Voltage above background to register signal
float rise_time = 2;              // Must see signal this long for 0 -> 1 (ms)
float background_timeout = 5000;  // Max time to not calculate background (ms)
//}

// Initialization
int state = 0;                 // 0 for no signal; 1 for signal
float background = 0;          // Tracks background level
bool high_now = false;         // Whether the instaneous signal is high
int last_low = 0;              // Time (ms) of last observed low
int last_high = 0;             // Time (ms) of last observed high
int last_background = 0;       // Time (ms) of contributing background

// create a one pole filter to estimate background
FilterOnePole backgroundFilter(LOWPASS, filterFrequency);   
  
// the setup routine runs once when you press reset:
void setup() {
 
  // initialize serial communication at 19200 bits per second:
  Serial.begin(19200);

  // Initialize the on-board LED (will use it to show state)
  pinMode(LED_BUILTIN, OUTPUT);

}

// the loop routine runs over and over
void loop() {
  
  // read the input:
  int sensorValue = analogRead(SensorInputPin);
  int current_time = millis();
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);

  // Do accounting based on whether we are exceeding threshold
  if (sensor == 0){
    high_now = (voltage - background) > threshold;
  }
  else if (sensor == 1){
    high_now = abs(voltage - background) > threshold;
  }
  if (high_now) {
    last_high = current_time;
  }
  else {
    last_low = current_time;
  }

  // Determine the state 
  if (high_now) {
    if ((current_time - last_low) > rise_time) {
      state = 1;
    }
  }
  else {     // not high_now
    if ((current_time - last_high) > fall_time) {
      state = 0;
    }
  }

  // Track the background
  if ((state == 0) and not high_now){
    background = backgroundFilter.input(voltage);
    last_background = current_time;
    }
  else if (current_time - last_background > background_timeout){
    background = backgroundFilter.input(voltage);      
  }

  // Print out the system values
  Serial.print(voltage);
  Serial.print("   ");
  Serial.print(state);
  Serial.print("   ");
  Serial.println(background);

  // Control the digital outputs
  if (state == 1){digitalWrite(LED_BUILTIN, HIGH);}
  else {digitalWrite(LED_BUILTIN, LOW);}
  
}

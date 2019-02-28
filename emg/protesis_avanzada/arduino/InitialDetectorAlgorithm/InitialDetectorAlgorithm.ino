/*
  InitialDetectorAlgorithm

  Development code for figuring out how to provide a robust signal
  from the Protesis Avanzada EMG sensor

  Bryn Davis

*/

#include <Filters.h>   // From https://github.com/JonHub/Filters

#define SensorInputPin A5 // input pin number

// Parameters
float threshold = .5;          // Threshold to compare to
float filterFrequency = 0.2;   // Change rate to be considered background (Hz)
float rise_time = 100;         // Must see signal this long for 0 -> 1 (ms)
float fall_time = 1000;        // Signal must be low this long for 1 -> 0 (ms)

// Initialization
int state = 0;                 // 0 for no signal; 1 for signal
float background = 0;          // Tracks background level
bool high_now = false;         // Whether the instaneous signal is high
int last_low = 0;              // Time (ms) of last observed low
int last_high = 0;             // Time (ms) of last observed high

// create a one pole filter to estimate background
FilterOnePole backgroundFilter(LOWPASS, filterFrequency);   
  
// the setup routine runs once when you press reset:
void setup() {
 
  // initialize serial communication at 9600 bits per second:
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
  if ((voltage - background) > threshold) {
    last_high = millis();
    high_now = true;
  }
  else {
    last_low = millis();
    high_now = false;
  }

  // Determine the state and track the background
  if (high_now) {
    if ((current_time - last_low) > rise_time) {
      state = 1;
    }
  }
  else {     // not high_now
    if ((current_time - last_high) > fall_time) {
      state = 0;
    }
    // Update the background filter
    if (state == 0){
       background = backgroundFilter.input(voltage);
    }
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

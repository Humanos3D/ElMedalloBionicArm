/*
  ReadAnalogVoltage

  Reads an analog input on pin 5, converts it to voltage, and prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/ReadAnalogVoltage

  Bryn simply modified the analog pin to use a pass through on the FlexVolt
*/

#include <Filters.h>   // From https://github.com/JonHub/Filters

#define SensorInputPin A5 // input pin number

float threshold = .5;          // Threshold to compare to
int state = 0;                 // 0 for no signal; 1 for signal
float filterFrequency = 0.2;   // Change rate to be considered background (Hz)

// create a one pole filter to estimate background
FilterOnePole backgroundFilter(LOWPASS, filterFrequency);   
float Background = 0;
  
// the setup routine runs once when you press reset:
void setup() {
 
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  
  // read the input:
  int sensorValue = analogRead(SensorInputPin);
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);

  // print out the value you read:
  Serial.print(voltage);
  Serial.print("   ");
  if ((voltage - Background) > threshold) {state = 1;}
  else {
    state = 0;
    // Update the background filter
    Background = backgroundFilter.input(voltage);
  }
  Serial.print(state);
  Serial.print("   ");
  Serial.println(Background);
}

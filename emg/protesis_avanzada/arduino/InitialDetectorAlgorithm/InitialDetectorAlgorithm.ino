/*
  ReadAnalogVoltage

  Reads an analog input on pin 5, converts it to voltage, and prints the result to the Serial Monitor.
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/ReadAnalogVoltage

  Bryn simply modified the analog pin to use a pass through on the FlexVolt
*/

#include "EMGFilters.h"

#define SensorInputPin A5 // input pin number

EMGFilters paFilter;
int sampleRate = SAMPLE_FREQ_500HZ;
int humFreq = NOTCH_FREQ_60HZ;

float threshold = 1.5;     // Threshold to compare to
int state = 0;           // 0 for no signal; 1 for signal

// the setup routine runs once when you press reset:
void setup() {
  // get the filter going
  paFilter.init(sampleRate, humFreq, false, false, true);
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  
  // read the input:
  int sensorValue = analogRead(SensorInputPin);
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);

  // Update the filter
  int DataAfterFilter = paFilter.update(sensorValue);
  float FilteredData = DataAfterFilter * (5.0 / 1023.0);
  
  // print out the value you read:
  Serial.print(voltage);
  Serial.print("   ");
  if (voltage > threshold) {state = 1;}
  else {state = 0;}
  Serial.print(state);
  Serial.print("   ");
  Serial.println(FilteredData);
}

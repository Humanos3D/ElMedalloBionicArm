/*

  2SensorDetectorAlgorithm



  Development code for figuring out how to provide a robust signal

  from either EMG sensor



  For a detailed discussion of functionality and defining parameters, 

  see https://github.com/enable-medellin/robotic-arm/wiki/Initial-EMG-Algorithm



  Bryn Davis,



*/



#include <Filters.h>         // From https://github.com/JonHub/Filters



#define SensorInputPin1 A5    // Input pin number for sensor 1

#define DigitalOutPin1 13     // Output pin for sensor 1

#define SensorInputPin2 A0    // Input pin number for sensor 2

#define DigitalOutPin2 12     // Output pin for sensor 2


// Setup parameters

int sensor_1_type = 1;                   // 0 for Protesis Avanzada; 1 for OYMotion

int sensor_2_type = 1;                   // 0 for Prostesis Avansada; 1 for OYMotion

int sensor_1_position = 0;               // 0 for bicep; 1 for forearm

int sensor_2_position = 1;               // 0 for bicep; 1 for forearm

int debug_signals = 1;                   // 1 to show signals over serial; 0 to not

int sensor_number = 2;                   // Number of sensors (1 or 2)


// Fixed parameters for all sensor positions and types

float background_frequency = 0.2; // Change rate to be considered background (Hz)

float ceiling_frequency = 150;    // Highest expected frequency (Hz)

float fall_time = 1000;           // Signal must be low this long for 1 -> 0 (ms)



// Variable parameters we'll change in setup for sensor position and type

float threshold1;                     // Voltage above background to register signal

float threshold2;

float rise_time1;                     // Must see signal this long for 0 -> 1 (ms)

float rise_time2;

float background_timeout1;            // Max time to not calculate background (ms)

float background_timeout2;

float threshold_oy_bicep = .25;            

float rise_time_oy_bicep = 2;      

float background_timeout_oy_bicep = 5000;  

float threshold_oy_forearm = .25;            

float rise_time_oy_forearm = 2;      

float background_timeout_oy_forearm = 5000; 

float threshold_pa_bicep = .5;         

float rise_time_pa_bicep = 100;    

float background_timeout_pa_bicep = 5000; 

float threshold_pa_forearm = .5;         

float rise_time_pa_forearm = 100;    

float background_timeout_pa_forearm = 5000; 



// Initialization

int state1 = 0;                 // 0 for no signal; 1 for signal

int state2 =0;

float background1 = 0;          // Tracks background level

float background2 = 0;

bool high_now1 = false;         // Whether the instaneous signal is high

bool high_now2 = false; 

int last_low1 = 0;              // Time (ms) of last observed low

int last_low2 = 0;

int last_high1 = 0;             // Time (ms) of last observed high

int last_high2 = 0;

int last_background1 = 0;       // Time (ms) of contributing background

int last_background2 = 0;

int current_time1 = 0;          // Tracking loop time (ms)

int current_time2 = 0;

int previous_time1 = 0;         // Stores the previous time for rate calculations

float emg_signal1;              // Current signal 

float emg_signal_2;             //Current signal of second sensor


// create a one pole filter to estimate background

FilterOnePole backgroundFilter(LOWPASS, background_frequency); 



// create a filter to remove high frequency noise  

FilterOnePole lowpassFilter(LOWPASS, ceiling_frequency); 

  

// the setup routine runs once when you press reset:

void setup() {

 

  // initialize serial communication at 19200 bits per second:

  Serial.begin(38400);




  // Initialize a digital output for hand-off to robotics (use LED in practice)

  pinMode(DigitalOutPin1, OUTPUT); 
  pinMode(DigitalOutPin2, OUTPUT);


  // Set sensor and position specific parameters for each sensor

  if ((sensor_1_type == 0)&&(sensor_1_position == 0)){

    threshold1 = threshold_pa_bicep;

    rise_time1 = rise_time_pa_bicep; 

    background_timeout1 = background_timeout_pa_bicep;

  }

  else if ((sensor_1_type == 0)&&(sensor_1_position == 1)){

    threshold1 = threshold_pa_forearm;

    rise_time1 = rise_time_pa_forearm; 

    background_timeout1 = background_timeout_pa_forearm; 

   }
    
  else if ((sensor_1_type == 1)&&(sensor_1_position == 0)){

    threshold1 = threshold_oy_bicep;

    rise_time1 = rise_time_oy_bicep; 

    background_timeout1 = background_timeout_oy_bicep; 

   }  
   else if ((sensor_1_type == 1)&&(sensor_1_position == 1)){

    threshold1 = threshold_oy_forearm;

    rise_time1 = rise_time_oy_forearm; 

    background_timeout1 = background_timeout_oy_forearm; 

    }

  if ((sensor_2_type == 0)&&(sensor_2_position == 0)){

    threshold2 = threshold_pa_bicep;

    rise_time2 = rise_time_pa_bicep; 

    background_timeout2 = background_timeout_pa_bicep;

  }

  else if ((sensor_2_type == 0)&&(sensor_2_position == 1)){

    threshold2 = threshold_pa_forearm;

    rise_time2 = rise_time_pa_forearm; 

    background_timeout2 = background_timeout_pa_forearm; 

   }
    
  else if ((sensor_2_type == 1)&&(sensor_2_position == 0)){

    threshold2 = threshold_oy_bicep;

    rise_time2 = rise_time_oy_bicep; 

    background_timeout2 = background_timeout_oy_bicep; 

   }  
   else if ((sensor_2_type == 1)&&(sensor_2_position == 1)){

    threshold2 = threshold_oy_forearm;

    rise_time2 = rise_time_oy_forearm; 

    background_timeout2 = background_timeout_oy_forearm; 

    }
}



// the loop routine runs over and over

void loop() {

  
//Sensor 1:

// read the input

int sensorValue1 = analogRead(SensorInputPin1);
  current_time1 = millis();



  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):

  float voltage1 = sensorValue1 * (5.0 / 1023.0);



  // do low pass filtering

 
  if (ceiling_frequency == 0){     // Do nothing

    emg_signal1 = voltage1;

  }

  else {                           // Filter out high frequencies

    emg_signal1 = lowpassFilter.input(voltage1);    
  }



  // Do accounting based on whether we are exceeding threshold

  if (sensor_1_type == 0){

    high_now1 = (emg_signal1 - background1) > threshold1;
  }

  else if (sensor_1_type == 1){

    high_now1 = abs(emg_signal1 - background1) > threshold1;
  }

  if (high_now1) {

    last_high1 = current_time1;

  }

  else {

    last_low1 = current_time1;

  }


  // Determine the state 

  if (high_now1) {

    if ((current_time1 - last_low1) > rise_time1) {

      state1 = 1;

    }

  }

  else {     // not high_now

    if ((current_time1 - last_high1) > fall_time) {

      state1 = 0;

    }

  }
  // Track the background

  if ((state1 == 0) and not high_now1){

    background1 = backgroundFilter.input(emg_signal1);

    last_background1 = current_time1;

    }

  else if (current_time1 - last_background1 > background_timeout1){

    background1 = backgroundFilter.input(emg_signal1);      

  }

//code for the second sensor if two grips are required (only runs when snesor_number ==2)

if (sensor_number == 2){

//read the input of sensor 2
  
 int sensorValue2 = analogRead(SensorInputPin2);
 current_time2 = millis();

// Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):

 float voltage2 = sensorValue2 * (5.0 / 1023.0);
 

  //low pass filtering
  
  if (ceiling_frequency == 0){     // Do nothing
        emg_signal_2 = voltage2;
  }
 else {                           // Filter out high frequencies
    
    emg_signal_2 = lowpassFilter.input(voltage2);
  }

  // Do accounting based on whether we are exceeding threshold
  
if (sensor_2_type == 0){
  
    high_now2 = (emg_signal_2 - background2) > threshold2;
  }

  else if (sensor_2_type == 1){

    high_now2 = abs(emg_signal_2 - background2) > threshold2;
  }

  if (high_now2) {

    last_high2 = current_time2;

  }

  else {

    last_low2 = current_time2;

  }


  // Determine the state 


  if (high_now2) {

    if ((current_time2 - last_low2) > rise_time2) {

      state2 = 1;

    }

  }

  else {     // not high_now

    if ((current_time2 - last_high2) > fall_time) {

      state2 = 0;

    }

  }


// Track the background

  if ((state2 == 0) and not high_now2){

    background2 = backgroundFilter.input(emg_signal_2);

    last_background2 = current_time2;

    }

  else if (current_time2 - last_background2 > background_timeout2){

    background2 = backgroundFilter.input(emg_signal_2);      

  }

}
  
else {}    //dont bother about the second sensor

  
  // Print out the signal values of sesnsor 2

  if (debug_signals == 1){

     Serial.print(emg_signal1);

     Serial.print("   ");

     Serial.print(state1);

     Serial.print("   ");

  // Serial.print(high_now);

  // Serial.print("   ");

     Serial.print(background1);

     Serial.print("   ");

  }

  //print out signal values of sensor 2
  
if ((debug_signals == 1) && (sensor_number == 2)){

  

     Serial.print(emg_signal_2);

     Serial.print("   ");

     Serial.print(state2);

     Serial.print("   ");

  // Serial.print(high_now2);

  // Serial.print("   ");

     Serial.print(background2);

     Serial.print("   ");
}

  // Track the speed (in hundreds of Hz to keep similar scale)

  float rate = 10. / (current_time1 - previous_time1);

  previous_time1 = current_time1;

  Serial.println(rate);

  

  // Control the digital outputs linked to sesnsor 1

  if (state1 == 1){

    digitalWrite(DigitalOutPin1, HIGH);

  }

  else {

    digitalWrite(DigitalOutPin1, LOW);

  }

 // Control the digital outputs linked to sesnsor 1. Here, the digital pin for sensor2 only fires if esnsor1 is not exceding the threshold.

  
 if ((state2 == 1)&&(state1 == 0)){

    digitalWrite(DigitalOutPin2, HIGH);

  }

  else {

    digitalWrite(DigitalOutPin2, LOW);

  }

  

}

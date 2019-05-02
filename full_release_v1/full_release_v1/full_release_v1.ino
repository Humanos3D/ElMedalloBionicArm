/*
  First full release for the robotic arm at e-NABLE Medellin

  For a detailed discussion of functionality and defining parameters for the EMG system,
  see https://github.com/enable-medellin/robotic-arm/wiki/Initial-EMG-Algorithm

  Completed ~April 2019

  EMGSetup() and EMGLoop() written by Bryn David and  Ben Complin
  Everything else written by Mark Walbran
*/

// Include the libraries
#include <Servo.h>
#include <Filters.h>         // From https://github.com/JonHub/Filters

// Define pins
#define servoPin 9
#define servoPin2 10
#define lockSwitchPin 11
#define buttonPin A1
#define SensorInputPin A0    // Input pin number

// Set constants
const int OPEN_POS = 0;
const int CLOSED_POS = 140;
const int OPEN_POS2 = 0;
const int CLOSED_POS2 = 180;
const int servoDelayTime = 500;
const int lockSwitchDelayTime = 20;
const int lockSwitchCounterLimit = 30;

// Initialise variables
boolean state = 0;                 // 0 for no signal; 1 for signal
boolean prevState = 0;
boolean motorState = 0;            // 0 for open hand; 1 for closed hand
boolean lockSwitchState = 0;
int motorValue = OPEN_POS;
int motorValue2 = OPEN_POS2;
int lockSwitchCounter = 0;

// Setup parameters
boolean buttonFlag = 0; // 0 to use EMG sensors; 1 to use button
boolean lockSwitchFlag = 0; // 0 to ignore lockswitch; 1 to use lockswitch
boolean motorFlag1 = 1; // 0 to disable motor; 1 to enable
boolean motorFlag2 = 0; // 0 to disable motor; 1 to enable

// Debugging options - 1 to show signals over serial interface (to viewed by Serial Monitor or Serial Plotter)
boolean EMGDebugging = 0;
boolean lockSwitchDebugging = 0;


// Create servo objects
Servo Servo1;
Servo Servo2;

// Function declaration
void EMGSetup();
void EMG();
void servos();
void readLockSwitch();

// ---------EMG setup----------
// Setup parameters
int sensor = 1;                   // 0 for Protesis Avanzada; 1 for OYMotion

// Fixed parameters
float background_frequency = 0.2; // Change rate to be considered background (Hz)
float ceiling_frequency = 150;    // Highest expected frequency (Hz)
float fall_time = 1000;           // Signal must be low this long for 1 -> 0 (ms)

// Variable parameters we'll change in setup
float threshold;                     // Voltage above background to register signal
float rise_time;                     // Must see signal this long for 0 -> 1 (ms)
float background_timeout;            // Max time to not calculate background (ms)
float threshold_oy = .25;
float rise_time_oy = 2;
float background_timeout_oy = 5000;
float threshold_pa = .5;
float rise_time_pa = 100;
float background_timeout_pa = 5000;

// Initialization
float background = 0;          // Tracks background level
bool high_now = false;         // Whether the instaneous signal is high
int last_low = 0;              // Time (ms) of last observed low
int last_high = 0;             // Time (ms) of last observed high
int last_background = 0;       // Time (ms) of contributing background
int current_time = 0;          // Tracking loop time (ms)
int previous_time = 0;         // Stores the previous time for rate calculations
float emg_signal;              // Current signal

// create a one pole filter to estimate background
FilterOnePole backgroundFilter(LOWPASS, background_frequency);

// create a filter to remove high frequency noise
FilterOnePole lowpassFilter(LOWPASS, ceiling_frequency);

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 38400 bits per second:
  Serial.begin(38400);

  // Initialize the on-board LED (will use it to show state)
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialise servos
  if (motorFlag1) {
    Servo1.attach(servoPin);
    Servo1.write(motorValue);
  }

  if (motorFlag2) {
    Servo2.attach(servoPin2);
    Servo2.write(motorValue2);
  }

  // Initialise button and lockSwitch
  pinMode(buttonPin, INPUT);
  pinMode(lockSwitchPin, INPUT);

  // Rung EMG setup tasks
  EMGSetup();
}

// the loop routine runs over and over
void loop() {
  // Read in lockswitch values and debounce
  if (lockSwitchFlag) {
    readLockSwitch();
  }

  // If lockswitch is off then then process input data and activate motors accordingly
  // If lockswitch is on, do nothing
  if (lockSwitchCounter < lockSwitchCounterLimit) {
    // Save previous state
    prevState = state;

    // Read in new state from either button or EMG sensor(s)
    if (buttonFlag) {
      state = digitalRead(buttonPin);
    } else {
      EMG();
    }

    // Print state values to onbaord LED for debugging purposes
    if (state) {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
      digitalWrite(LED_BUILTIN, LOW);
    }

    // Check EMG signal state, and if we are on a rising edge then toggle motor state
    if ((!prevState) && (state)) {
      servos();
    }
  }
}

void EMGSetup() {
  // Set sensor specific parameters (TODO - Figure out how to do this only once)
  if (sensor) {
    threshold = threshold_oy;
    rise_time = rise_time_oy;
    background_timeout = background_timeout_oy;
  } else {
    threshold = threshold_pa;
    rise_time = rise_time_pa;
    background_timeout = background_timeout_pa;
  }
}

void EMG() {
  // read the input:
  int sensorValue = analogRead(SensorInputPin);
  current_time = millis();

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);

  // do low pass filtering
  if (ceiling_frequency == 0) {    // Do nothing
    emg_signal = voltage;
  }
  else {                           // Filter out high frequencies
    emg_signal = lowpassFilter.input(voltage);
  }

  // Do accounting based on whether we are exceeding threshold
  if (sensor == 0) {
    high_now = (emg_signal - background) > threshold;
  }
  else if (sensor == 1) {
    high_now = abs(emg_signal - background) > threshold;
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
  if ((state == 0) and not high_now) {
    background = backgroundFilter.input(emg_signal);
    last_background = current_time;
  }
  else if (current_time - last_background > background_timeout) {
    background = backgroundFilter.input(emg_signal);
  }

  // Track the speed (in hundreds of Hz to keep similar scale)
  float rate = 10. / (current_time - previous_time);
  previous_time = current_time;

  // Print out the signal values
  if (EMGDebugging == 1) {
    Serial.print(emg_signal);
    Serial.print("   ");
    Serial.print(state);
    Serial.print("   ");
    Serial.print(background);
    Serial.print("   ");

    Serial.println(rate);
  }
}

void servos() {
  // Print messages to serial for debugging purposes
  if (buttonFlag) {
    Serial.println("Button Pushed");
  } else {
    Serial.println("EMG signal rising edge");
  }

  // Toggle hand position
  if (motorState) { // If motor state = 1 (hand currently closed) then open hand
    motorValue = OPEN_POS;
    motorValue2 = OPEN_POS2;
    Serial.println("Opening hand..."); // Debugging
    motorState = 0;
  } else { // If motor state = 0 (hand currently open) then close hand
    motorValue = CLOSED_POS;
    motorValue2 = CLOSED_POS2;
    Serial.println("Closing hand..."); // Debugging
    motorState = 1;
  }

  // Write to active motors
  if (motorFlag1){
    Servo1.write(motorValue);
  }
  
  if (motorFlag2) {
    Servo2.write(motorValue2);
  }
  
  delay(servoDelayTime);
}

void readLockSwitch() {
  lockSwitchState = digitalRead(lockSwitchPin);

  // Print out lockswitch values
  if (lockSwitchDebugging) {
    Serial.print("LockSwitch State: ");
    Serial.println(lockSwitchState);
  }

  // If lockswitch is switched on (low in this case), then increase debouncing counter until it reaches the limit
  if (!lockSwitchState) {
    if (lockSwitchCounter < lockSwitchCounterLimit) {
      lockSwitchCounter++;
    } else if (lockSwitchCounter == lockSwitchCounterLimit) {
      Serial.println("Lock Switch on");
      digitalWrite(LED_BUILTIN, HIGH);
      lockSwitchCounter++;
    }
    delay(lockSwitchDelayTime);

    // If lock switch turned off then reset counter to 0
  } else {
    if (lockSwitchCounter > lockSwitchCounterLimit) {
      Serial.println("Lock Switch off");
      digitalWrite(LED_BUILTIN, LOW);
    }
    lockSwitchCounter = 0;
  }
}

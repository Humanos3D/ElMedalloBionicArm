/*
  InitialDetectorAlgorithm

  Development code for figuring out how to provide a robust signal
  from either EMG sensor

  For a detailed discussion of functionality and defining parameters,
  see https://github.com/enable-medellin/robotic-arm/wiki/Initial-EMG-Algorithm

  Bryn Davis

*/

// --------SERVO-CODE---------
// Include the Servo library
#include <Servo.h>

// Define pins and constants
const int servoPin = 11;
const int servoPin2 = 10;
const int lockSwitchPin = 9;
const int buttonPin = A1;
const int OPEN_POS = 0;
const int CLOSED_POS = 180;
const int OPEN_POS2 = 0;
const int CLOSED_POS2 = 180;
const int servoDelayTime = 500;

// Lock switch debouncing constants
const int lockSwitchDelayTime = 20;
const int lockSwitchCounterLimit = 30;
int lockSwitchCounter = 0;

// Initialise variables
boolean stateFlag = LOW;
boolean prevState = LOW;
int motorValue = OPEN_POS;
int motorValue2 = OPEN_POS2;

// Parameters to be set
boolean buttonFlag = 1; // 1 to use button, 0 to use EMG sensors
boolean lockSwitchFlag = 0; // 1 to use lockswitch, 0 to ignore

// Create a servo object
Servo Servo1;
Servo Servo2;

void servoSetup();
void servoLoop();

// ----------EMG-CODE----------
#include <Filters.h>         // From https://github.com/JonHub/Filters

#define SensorInputPin A0    // Input pin number
#define DigitalOutPin 11     // Output pin

// Setup parameters
int sensor = 1;                   // 0 for Protesis Avanzada; 1 for OYMotion
int debug_signals = 0;            // 1 to show signals over serial

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
int state = 0;                 // 0 for no signal; 1 for signal
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
  servoSetup();

  // initialize serial communication at 19200 bits per second:
  Serial.begin(38400);

  // Initialize the on-board LED (will use it to show state)
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize a digital output for hand-off to robotics
  pinMode(DigitalOutPin, OUTPUT);

  // Set sensor specific parameters (TODO - Figure out how to do this only once)
  if (sensor == 0) {
    threshold = threshold_pa;
    rise_time = rise_time_pa;
    background_timeout = background_timeout_pa;
  }
  else if (sensor == 1) {
    threshold = threshold_oy;
    rise_time = rise_time_oy;
    background_timeout = background_timeout_oy;
  }

}

// the loop routine runs over and over
void loop() {

  if (buttonFlag == LOW) {
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
    if (debug_signals == 1) {
      Serial.print(emg_signal);
      Serial.print("   ");
      Serial.print(state);
      Serial.print("   ");
      // Serial.print(high_now);
      // Serial.print("   ");
      Serial.print(background);
      Serial.print("   ");

      Serial.println(rate);
    }
  } else {
    state = digitalRead(buttonPin);
  }

  servoLoop();

}

void servoSetup() {
  // Initialise servo and button mode
  Servo1.attach(servoPin);
  Servo1.write(motorValue);
  Servo2.attach(servoPin2);
  Servo2.write(motorValue2);

  pinMode(buttonPin, INPUT);

  pinMode(lockSwitchPin, INPUT);
}

void servoLoop() {
  // LockSwitch Debouncing
  if (lockSwitchFlag == HIGH) {
    if (digitalRead(lockSwitchPin) == HIGH) {
      if (lockSwitchCounter <= lockSwitchCounterLimit) {
        if (lockSwitchCounter == lockSwitchCounterLimit) { // If lock switch has been on for lockSwitchDelayTime*lockSwitchCounterLimit then activate LED etc.
          Serial.println("Lock Switch on");
          digitalWrite(LED_BUILTIN, HIGH);
        }

        lockSwitchCounter++;
        delay(lockSwitchDelayTime);
      }
    } else {
      if (lockSwitchCounter >= lockSwitchCounterLimit) {
        Serial.println("Lock Switch off");
        digitalWrite(LED_BUILTIN, LOW);
      }
      lockSwitchCounter = 0;
    }
  }

  if (lockSwitchCounter < lockSwitchCounterLimit) {
    // Toggle motor position on EMG signal rising edge unless lockSwitch is on
    // Control the digital outputs for debugging purposes
    if (state == 1) {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(DigitalOutPin, HIGH);
    }
    else {
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(DigitalOutPin, LOW);
    }

    // If lock switch off: Check EMG signal state, and if we are on a rising edge then toggle motor state
    if ((prevState == LOW) && (state == HIGH)) {
      if (buttonFlag == 1) {
        Serial.println("Button Pushed"); // Debugging
      } else {
        Serial.println("EMG rising edge");
      }
      if (stateFlag == LOW) {
        motorValue = CLOSED_POS;
        motorValue2 = CLOSED_POS2;
        Servo1.write(motorValue);
        Servo2.write(motorValue2);
        Serial.println("Closing..."); // Debugging
        stateFlag = HIGH;
      } else if (stateFlag == HIGH) {
        motorValue = OPEN_POS;
        motorValue2 = OPEN_POS2;
        Servo1.write(motorValue);
        Servo2.write(motorValue2);
        Serial.println("Opening..."); // Debugging
        stateFlag = LOW;
      }
      delay(servoDelayTime);
    }
  }

  prevState = state;
}

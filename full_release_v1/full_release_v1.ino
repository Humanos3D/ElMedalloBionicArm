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
#define servoPin2 11
#define lockSwitchPin 10
#define buttonPin A1
#define sensorPin A0          // EMG sensor 1, corresponds to servo 1 closing first
#define sensorPin2 A2         // EMG sensor 2, corresponds to servo 2 closing first

// Set constants
const int OPEN_POS = 0;
const int CLOSED_POS_GRIP1 = 180; // Motor angles in degrees
const int CLOSED_POS_GRIP2 = 180;
const int OPEN_POS2 = 180;
const int CLOSED_POS2_GRIP1 = 0;
const int CLOSED_POS2_GRIP2 = 0;
const int DELAY_TIME_GRIP1 = 300;  // Delay in milliseconds
const int DELAY_TIME_GRIP2 = 300;
const int SERVO_TIMEOUT_TIME = 1500; // Time after which the servos will stop trying to turn and the arm will be available for another grip
const int lockSwitchDelayTime = 20; // Lockswitch debouncing constants
const int lockSwitchCounterLimit = 30; // Lockswitch debouncing constants

// Setup parameters
boolean buttonFlag = 0; // 0 to use EMG sensors; 1 to use button
boolean lockSwitchFlag = 1; // 0 to have lockswitch toggle between which EMG signal the button simulates; 1 to use lockswitch as a lock switch
boolean motorFlag1 = 1; // 0 to disable motor; 1 to enable
boolean motorFlag2 = 1; // 0 to disable motor; 1 to enable

// Initialise variables
boolean state = 0;                 // 0 for no signal; 1 for signal
boolean state2 = 0;
boolean prevState = 0;
boolean prevState2 = 0;
boolean motorState = 0;            // 0 for open hand; 1 for closed hand
boolean lockSwitchState = 0;
int motorValue = OPEN_POS;
int motorValue2 = OPEN_POS2;
int lockSwitchCounter = 0;  // Debouncing

// Debugging options - 1 to show signals over serial interface (to viewed by Serial Monitor or Serial Plotter)
boolean EMGDebugging = 1;
boolean lockSwitchDebugging = 0;


// Create servo objects
Servo servo;
Servo servo2;

// Function declaration
void EMGSetup();
void EMG();
void servos(int gripNumber);
void readLockSwitch();

// ---------EMG setup----------
// Setup parameters
boolean sensorType = 1;                   // 0 for Protesis Avanzada; 1 for OYMotion
boolean sensorType2 = 1;                   // 0 for Protesis Avanzada; 1 for OYMotion
int sensorFlag = 2;                    // 0 to disable sensor; 1 for bicep; 2 for foreearm
int sensorFlag2 = 1;                   // 0 for disable sensor; 1 for bicep; 2 for forearm

// Fixed parameters
float background_frequency = 0.2; // Change rate to be considered background (Hz)
float ceiling_frequency = 150;    // Highest expected frequency (Hz)
float fall_time = 1000;           // Signal must be low this long for 1 -> 0 (ms)

// Variable parameters we'll change in setup
float threshold;                     // Voltage above background to register signal
float threshold2;                     // Voltage above background to register signal
float rise_time;                     // Must see signal this long for 0 -> 1 (ms)
float rise_time2;                     // Must see signal this long for 0 -> 1 (ms)
float background_timeout;            // Max time to not calculate background (ms)
float background_timeout2;            // Max time to not calculate background (ms)
float threshold_oy_forearm = .25;
float rise_time_oy_forearm = 2;
float background_timeout_oy_forearm = 5000;
float threshold_pa_forearm = .5;
float rise_time_pa_forearm = 100;
float background_timeout_pa_forearm = 5000;
float threshold_oy_bicep = .25;
float rise_time_oy_bicep = 2;
float background_timeout_oy_bicep = 5000;
float threshold_pa_bicep = .5;
float rise_time_pa_bicep = 100;
float background_timeout_pa_bicep = 5000;

// Initialization
float background = 0;          // Tracks background level
bool high_now = false;         // Whether the instaneous signal is high
int last_low = 0;              // Time (ms) of last observed low
int last_high = 0;             // Time (ms) of last observed high
int last_background = 0;       // Time (ms) of contributing background
int current_time = 0;          // Tracking loop time (ms)
int previous_time = 0;         // Stores the previous time for rate calculations
float emg_signal;              // Current signal

float background2 = 0;          // Tracks background level
bool high_now2 = false;         // Whether the instaneous signal is high
int last_low2 = 0;              // Time (ms) of last observed low
int last_high2 = 0;             // Time (ms) of last observed high
int last_background2 = 0;       // Time (ms) of contributing background
int current_time2 = 0;          // Tracking loop time (ms)
int previous_time2 = 0;         // Stores the previous time for rate calculations
float emg_signal2;              // Current signal

// create one pole filters to estimate background
FilterOnePole backgroundFilter(LOWPASS, background_frequency);
FilterOnePole backgroundFilter2(LOWPASS, background_frequency);

// create filters to remove high frequency noise
FilterOnePole lowpassFilter(LOWPASS, ceiling_frequency);
FilterOnePole lowpassFilter2(LOWPASS, ceiling_frequency);

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 38400 bits per second:
  Serial.begin(38400);

  // Initialize the on-board LED (will use it to show state)
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialise servos
  if (motorFlag1) {
    servo.attach(servoPin);
    servo.write(motorValue);
  }

  if (motorFlag2) {
    servo2.attach(servoPin2);
    servo2.write(motorValue2);
  }

  // Initialise button and lockSwitch
  pinMode(buttonPin, INPUT);
  pinMode(lockSwitchPin, INPUT);

  // Rung EMG setup tasks
  EMGSetup();

  // Detach servos
  delay(SERVO_TIMEOUT_TIME);
  servo.detach();
  servo2.detach();
}

// the loop routine runs over and over
void loop() {
  // Read in lockswitch values and debounce
  readLockSwitch();

  // If lockswitch is off then then process input data and activate motors accordingly
  // If lockswitch is on & lockSwitchFlag is high, do nothing
  if ((!lockSwitchFlag) || (lockSwitchCounter < lockSwitchCounterLimit)) {
    // Save previous state
    prevState = state;
    prevState2 = state2;

    // Read in new state from either button or EMG sensor(s)
    if (buttonFlag) {
      if (lockSwitchFlag) {
        state = digitalRead(buttonPin);
      } else {
        if (lockSwitchCounter < lockSwitchCounterLimit) {
          state = digitalRead(buttonPin);
        } else {
          state2 = digitalRead(buttonPin);
        }
      }
    } else {
      if (sensorFlag) {
        EMG();
      }

      if (sensorFlag2) {
        EMG2();
      }
    }

    // Print state values to onbaord LED for debugging purposes
    if (state || state2) {
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }


    // Check EMG signal state, and if we are on a rising edge then toggle motor state
    // NOTE: EMG signal 1 has priority, if EMG signal 1 is on a rising edge then we will not check EMG signal 2
    if (!EMGDebugging) {
      if ((!prevState) && (state)) {
        // Print messages to serial for debugging purposes
        if (buttonFlag) {
          Serial.println("Button Pushed");
        } else {
          Serial.println("EMG signal rising edge");
        }
        servos(1);
      } else if ((!prevState2) && (state2)) {
        // Print messages to serial for debugging purposes
        if (buttonFlag) {
          Serial.println("Button Pushed 2");
        } else {
          Serial.println("EMG signal 2 rising edge");
        }
        servos(2);
      }
    }
  }
}

void EMGSetup() {
  // Set sensor specific parameters (TODO - Figure out how to do this only once)
  if (sensorType) {
    if (sensorFlag == 1) {
      threshold = threshold_oy_bicep;
      rise_time = rise_time_oy_bicep;
      background_timeout = background_timeout_oy_bicep;
    } else if (sensorFlag == 2) {
      threshold = threshold_oy_forearm;
      rise_time = rise_time_oy_forearm;
      background_timeout = background_timeout_oy_forearm;
    }
  } else {
    if (sensorFlag == 1) {
      threshold = threshold_pa_bicep;
      rise_time = rise_time_pa_bicep;
      background_timeout = background_timeout_pa_bicep;
    } else if (sensorFlag == 2) {
      threshold = threshold_pa_forearm;
      rise_time = rise_time_pa_forearm;
      background_timeout = background_timeout_pa_forearm;
    }
  }

  if (sensorType2) {
    if (sensorFlag == 1) {
      threshold2 = threshold_oy_bicep;
      rise_time2 = rise_time_oy_bicep;
      background_timeout2 = background_timeout_oy_bicep;
    } else if (sensorFlag == 2) {
      threshold2 = threshold_oy_forearm;
      rise_time2 = rise_time_oy_forearm;
      background_timeout2 = background_timeout_oy_forearm;
    }
  } else {
    if (sensorFlag == 1) {
      threshold2 = threshold_pa_bicep;
      rise_time2 = rise_time_pa_bicep;
      background_timeout2 = background_timeout_pa_bicep;
    } else if (sensorFlag == 2) {
      threshold2 = threshold_pa_forearm;
      rise_time2 = rise_time_pa_forearm;
      background_timeout2 = background_timeout_pa_forearm;
    }
  }
}

void EMG() {
  // read the input:
  int sensorValue = analogRead(sensorPin);
  current_time = millis();

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);

  // do low pass filtering
  if (!ceiling_frequency) {    // Do nothing
    emg_signal = voltage;
  }
  else {                           // Filter out high frequencies
    emg_signal = lowpassFilter.input(voltage);
  }

  // Do accounting based on whether we are exceeding threshold
  float trigger_threshhold;
  if (!sensorType) {
    trigger_threshhold = (emg_signal - background);
    high_now = trigger_threshhold > threshold;
  }
  else if (sensorType == 1) {
    trigger_threshhold = abs(emg_signal - background);
    high_now = trigger_threshhold > threshold;
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
  if (!state && !high_now) {
    background = backgroundFilter.input(emg_signal);
    last_background = current_time;
  }
  else if (current_time - last_background > background_timeout) {
    background = backgroundFilter.input(emg_signal);
  }

  // Track the speed (in hundreds of Hz to keep similar scale)
  float rate = 10. / (current_time - previous_time2);
  previous_time = current_time;

  // Print out the signal values
  if (EMGDebugging) {
    Serial.print(emg_signal);
    Serial.print("   ");
    Serial.print(state);
    Serial.print("   ");
    Serial.print(background);
    Serial.print("   ");
    Serial.print(trigger_threshhold);
    Serial.print("   ");

    Serial.print(rate);
  }
}

void EMG2() {
  // read the input:
  int sensorValue2 = analogRead(sensorPin2);
  current_time2 = millis();

  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage2 = sensorValue2 * (5.0 / 1023.0);

  // do low pass filtering
  if (!ceiling_frequency) {    // Do nothing
    emg_signal2 = voltage2;
  }
  else {                           // Filter out high frequencies
    emg_signal2 = lowpassFilter2.input(voltage2);
  }

  // Do accounting based on whether we are exceeding threshold
  float trigger_threshhold;
  if (!sensorType2) {
    trigger_threshhold = (emg_signal2 - background2);
    high_now2 = trigger_threshhold > threshold2;
  }
  else if (sensorType2 == 1) {
    trigger_threshhold = abs(emg_signal2 - background2);
    high_now2 = trigger_threshhold > threshold2;
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
  if (!state2 && !high_now2) {
    background2 = backgroundFilter2.input(emg_signal2);
    last_background2 = current_time2;
  }
  else if (current_time2 - last_background2 > background_timeout2) {
    background2 = backgroundFilter2.input(emg_signal2);
  }

  // Track the speed (in hundreds of Hz to keep similar scale)
  float rate2 = 10. / (current_time2 - previous_time2);
  previous_time2 = current_time2;

  // Print out the signal values
  if (EMGDebugging) {
    Serial.print(emg_signal2);
    Serial.print("   ");
    Serial.print(state2);
    Serial.print("   ");
    Serial.print(background2);
    Serial.print("   ");
    Serial.print(trigger_threshhold);
    Serial.print("   ");

    Serial.println(rate2);
  }
}

void servos(int gripNumber) {
  // Toggle hand position
  if (motorState) { // If motor state = 1 (hand currently closed) then open hand
    motorValue = OPEN_POS;
    motorValue2 = OPEN_POS2;
    Serial.println("Opening hand..."); // Debugging
    // Write to active motors
    if (motorFlag1) {
      servo.attach(servoPin);
      servo.write(motorValue);
    }
    if (motorFlag2) {
      servo2.attach(servoPin2);
      servo2.write(motorValue2);
    }
    delay(300);
    motorState = 0;
  } else { // If motor state = 0 (hand currently open) then close hand to specified grip
    if (gripNumber == 1) {
      motorValue = CLOSED_POS_GRIP1;
      motorValue2 = CLOSED_POS2_GRIP1;
      Serial.println("Closing hand Grip #1..."); // Debugging

      // Write to active motors
      if (motorFlag1) {
        servo.attach(servoPin);
        servo.write(motorValue);
      }
      delay(DELAY_TIME_GRIP1);
      if (motorFlag2) {
        servo2.attach(servoPin2);
        servo2.write(motorValue2);
      }
    } else if (gripNumber == 2) {
      motorValue = CLOSED_POS_GRIP2;
      motorValue2 = CLOSED_POS2_GRIP2;
      Serial.println("Closing hand Grip #2..."); // Debugging

      // Write to active motors
      if (motorFlag2) {
        servo2.attach(servoPin2);
        servo2.write(motorValue2);
      }
      delay(DELAY_TIME_GRIP2);
      if (motorFlag1) {
        servo.attach(servoPin);
        servo.write(motorValue);
      }
    }
    motorState = 1;
  }

  delay(SERVO_TIMEOUT_TIME);
  servo.detach();
  servo2.detach();
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

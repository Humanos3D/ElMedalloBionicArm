// Include the Servo library
#include <Servo.h>

// Define pins and constants
const int servoPin = 3;
const int buttonPin = 2;
const int OPEN_POS = 0;
const int CLOSED_POS = 180;
const int delayTime = 500;

// Initialise variables
boolean buttonState = LOW;
boolean buttonFlag = LOW;
boolean prevButtonState = LOW;
int motorValue = OPEN_POS;

// Create a servo object
Servo Servo1;

void setup() {
  // Initialise servo and button mode
  Servo1.attach(servoPin);
  Servo1.write(motorValue);
  pinMode(buttonPin, INPUT);
  
  //Debugging
  Serial.begin(9600);
}


void loop() {
  // Read button value
  buttonState = digitalRead(buttonPin);
  
  // Toggle motor position if button is pressed
  if ((prevButtonState == LOW) && (buttonState == HIGH)) {
    Serial.println("Button Pushed"); // Debugging
    if (buttonFlag == LOW) {
      motorValue = CLOSED_POS;
      Servo1.write(motorValue);
      Serial.println("Closing..."); // Debugging
      buttonFlag = HIGH;
    } else if (buttonFlag == HIGH) {
      motorValue = OPEN_POS;
      Servo1.write(motorValue);
      Serial.println("Opening..."); // Debugging
      buttonFlag = LOW;
    }
    delay(delayTime);
  }

  prevButtonState = buttonState;
}

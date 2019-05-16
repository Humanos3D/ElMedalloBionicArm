// SoftwareSerial - Version: Latest 
#include <SoftwareSerial.h>
#include <math.h>
#include <avr/sleep.h>
/*
Ultrasonic range finder with `reversing sensor - like` buzzer and an on/off push button
Author: Benno C
*/

#define IN4  4                                                      // Defines pin 5 as buzzer (called IN4 because I was using an LED in testing as it is less annoying)
#define SRF_TXRX         0x05                                       // Defines pin 5 to be used as RX and TX for SRF01
#define SRF_ADDRESS      0x01                                       // Address of the SFR01
#define GETSOFT          0x5D                                       // Byte to tell SRF01 we wish to read software version
#define GETRANGE         0x54                                       // Byte used to get range from SRF01
#define GETSTATUS        0x5F                                       // Byte used to get the status of the transducer

int val=0;                                                          // val is integer that is used for the time between beeps



SoftwareSerial srf01 = SoftwareSerial(SRF_TXRX, SRF_TXRX);          // Sets up software serial port for the SRF01

int pin_switch = 2;                                                 // Button pin initialised on pin 2/interrupt 0            
 
// variable used for the key press
volatile boolean keyPressed = false;

 
// variables used for the debounce
unsigned long timeNewKeyPress = 0;
unsigned long timeLastKeyPress = 0;
unsigned int timeDebounce = 10;

//variables used for the millis function for various buzzer beeps
long previousMillis = 0;
long interval = 50;
int IN4State =LOW;

void setup() {
  
  pinMode(4, OUTPUT);
  pinMode(pin_switch, INPUT); 

  Serial.begin(9600);                                     //comment out, only for testing sensor
  srf01.begin(9600);                                      
  srf01.listen();                                         // Make sure that the SRF01 software serial port is listening for data as only one software serial port can listen at a time
  
  delay(200);                                             // Waits some time to make sure everything is powered up
  
  byte softVer;
  SRF01_Cmd(SRF_ADDRESS, GETSOFT);                        // Request the SRF01 software version
  while (srf01.available() < 1);
    softVer = srf01.read();                               // Read software version from SRF01
  
}

void loop() {
    attachInterrupt( digitalPinToInterrupt(pin_switch), keyIsPressed, RISING );        //attach bush button as an interupt (every time as is detached in the interrupt function to act as a debounce to ensure one press = one action)


 // code for finding distance of object with the SRF01
 
    byte hByte, lByte, statusByte, b1, b2, b3;
  
  SRF01_Cmd(SRF_ADDRESS, GETRANGE);                       // Get the SRF01 to perform a ranging and send the data back to the arduino  
  while (srf01.available() < 2);
  hByte = srf01.read();                                   // Get high byte
  lByte = srf01.read();                                   // Get low byte
  int range = ((hByte<<8)+lByte);                         // Put them together

Serial.println("Range = ");                                  // Comment out for real life use... only useful for visualising on computer screen. 
  Serial.println(range, DEC);                                // Print range result to the screen
  Serial.println("  ");                                      // Print some spaces to the screen to make sure space direcly after the result is clear

// code for the buzzer beeping function
 
  if(range<=51){                                                                      // Tells the buzzer to beep when below 51cm
    val=20*(range);                                                                    // Val used for delay time between beep is 20 times that of the distance of the object
    unsigned long currentMillis = millis();
    if((currentMillis - previousMillis > val)&&(IN4State==LOW)&&(range>=1)){           // if the buzzer is off, the distance is 1cm or above & the calculated pause between beeps has been surpassed
      previousMillis = currentMillis;                                                  // reset milli timer
      IN4State = HIGH;                                                                 // change buzzer state from low to high
      }
     else if((currentMillis - previousMillis > interval)&&(IN4State==HIGH)&&(range>=1)){  // if the buzzer is on, the distance is 1cm or above & the beep has lasted for 50ms
      previousMillis = currentMillis;                                                      // reset milli timer
      IN4State = LOW;                                                                      // change buzzer state from High to Low
       }
     else if(range<1){                                                                      // if the distance is below 1cm
      IN4State = HIGH;                                                                      // Set buzzer state high
     }
     digitalWrite(IN4, IN4State);                                                            // turn on/off buzzer depending on the state set at that time. 
     }

// code for the button press interrupt to put arduino, beeping and range finding to sleep
    
    if (keyPressed)                                                                         // detect button press 
     {
          keyPressed = false;
          timeNewKeyPress = millis();                                                        // timer for debounce key press
 
          if ( timeNewKeyPress - timeLastKeyPress >= timeDebounce)                          // Debounce function, only a valid key press if the last one press is more than deboounce time ago 
          {
              sleepSetup();                                                                   // activate sleep interrupt to power down arduino, beeping and range finding
          }
          timeLastKeyPress = timeNewKeyPress;                                                 // reset time of button press
     }
     else{}                                                                                   // if the key press doesn`t occur, carry on with the code
}



// Detection code for the SRF01 (from the internet):

void SRF01_Cmd(byte Address, byte cmd){               // Function to send commands to the SRF01
  pinMode(SRF_TXRX, OUTPUT);
  digitalWrite(SRF_TXRX, LOW);                        // Send a 2ms break to begin communications with the SRF01                         
  delay(2);                                               
  digitalWrite(SRF_TXRX, HIGH);                            
  delay(1);                                                
  srf01.write(Address);                               // Send the address of the SRF01
  srf01.write(cmd);                                   // Send commnd byte to SRF01
  pinMode(SRF_TXRX, INPUT);
  int availbleJunk = srf01.available();               // As RX and TX are the same pin it will have recieved the data we just sent out, as we dont want this we read it back and ignore it as junk before waiting for useful data to arrive
  for(int x = 0;  x < availbleJunk; x++) byte junk = srf01.read();
  
}

// detetcing button press interrrupt:

void keyIsPressed()
{
   keyPressed = true;        
}


// Interrupt to send arduino to sleep:

void sleepSetup()
{
        sleep_enable();                                              
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);                                                   // choosing desired sleep mode... there are many but this is the most power saving
        attachInterrupt(digitalPinToInterrupt(pin_switch), pinInterrupt, RISING);              // allows button pin interrupt to be available during sleep mode
        digitalWrite(4,LOW);                                                                   // to ensure the buzzer is off when in sleep
        sleep_cpu();                                                                           // arduino sleeps

}     

// interrupt to wake arduino back up:

void pinInterrupt() //ISR
{ 
  sleep_disable();          // wake back up
  detachInterrupt(0);       // detaches button press interrupt to act as a debounce function
} 

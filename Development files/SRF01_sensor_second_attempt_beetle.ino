// SoftwareSerial - Version: Latest 
#include <SoftwareSerial.h>
/*
Ultrasonic range finder with `reversing sensor - like` buzzer and an on/off push button
Author: Benno C
*/

#define Buzzer  10                                                  // Defines pin 10 as buzzer
#define SRF_TXRX         0x09                                       // Defines pin 9 to be used as RX and TX for SRF01
#define SRF_ADDRESS      0x01                                       // Address of the SFR01
#define GETSOFT          0x5D                                       // Byte to tell SRF01 we wish to read software version
#define GETRANGE         0x54                                       // Byte used to get range from SRF01
#define GETSTATUS        0x5F                                       // Byte used to get the status of the transducer

int val=0;                                                          // val is integer that is used for the time between beeps



SoftwareSerial srf01 = SoftwareSerial(SRF_TXRX, SRF_TXRX);          // Sets up software serial port for the SRF01
    

//variables used for the millis function for various buzzer beeps
long previousMillis = 0;
long interval = 50;
int BuzzerState =LOW;

void setup() {
  
  pinMode(10, OUTPUT);

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
    if((currentMillis - previousMillis > val)&&(BuzzerState==LOW)&&(range>=1)){           // if the buzzer is off, the distance is 1cm or above & the calculated pause between beeps has been surpassed
      previousMillis = currentMillis;                                                  // reset milli timer
      BuzzerState = HIGH;                                                                 // change buzzer state from low to high
      }
     else if((currentMillis - previousMillis > interval)&&(BuzzerState==HIGH)&&(range>=1)){  // if the buzzer is on, the distance is 1cm or above & the beep has lasted for 50ms
      previousMillis = currentMillis;                                                      // reset milli timer
      BuzzerState = LOW;                                                                      // change buzzer state from High to Low
       }
     else if(range<1){                                                                      // if the distance is below 1cm
      BuzzerState = HIGH;                                                                      // Set buzzer state high
     }
     digitalWrite(Buzzer, BuzzerState);                                                            // turn on/off buzzer depending on the state set at that time. 
     }
                                                                                 
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

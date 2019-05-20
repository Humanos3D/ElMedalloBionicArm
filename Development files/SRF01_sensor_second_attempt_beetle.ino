// SoftwareSerial - Version: Latest 
#include <SoftwareSerial.h>
/*
Ultrasonic range finder with `reversing sensor - like` buzzer 
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
long interval = 50;                                                 //time length of buzzer beep
long shortRangeTimeout = 3000;                                      //if less than 1mm away from an object for **3s** or more, buzzer turns off
int BuzzerState =LOW;

//variables for the sensor glitch and transducer timeout to enable reset of system
long lastlock = 0;
long unlocktimeout = 2000;                                         //if transducer unlocked for **2s** or more, arduino resets
long lastlongrange = 0;
long lastshortrange = 0;
long sensorglitchtimeout = 10000;                                  //if range detetcted is over 30cm or under 30cm (one or the other, not inclusive) for **10s** or more, arduino resets

void setup() {
  
  pinMode(10, OUTPUT);                                      //setup buzzer as an output

  Serial.begin(9600);                                     //comment out, only for testing sensor
  srf01.begin(9600);                                      
  srf01.listen();                                         // Make sure that the SRF01 software serial port is listening for data as only one software serial port can listen at a time
  
  delay(200);                                             // Waits some time to make sure everything is powered up
  
  byte softVer;
  SRF01_Cmd(SRF_ADDRESS, GETSOFT);                        // Request the SRF01 software version
  while (srf01.available() < 1);
    softVer = srf01.read();                               // Read software version from SRF01
  
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop() {

 // code for finding distance of object with the SRF01
 
    byte hByte, lByte, statusByte, b1, b2, b3;
  
  SRF01_Cmd(SRF_ADDRESS, GETRANGE);                       // Get the SRF01 to perform a ranging and send the data back to the arduino  
  while (srf01.available() < 2);
  hByte = srf01.read();                                   // Get high byte
  lByte = srf01.read();                                   // Get low byte
  int range = ((hByte<<8)+lByte);                         // Put them together

Serial.println("Range = ");                                // Comment out for real life use... only useful for visualising on computer screen. 
Serial.println(range, DEC);                                // Print range result to the screen
Serial.println("  ");                                      // Print some spaces to the screen to make sure space direcly after the result is clear


//checking whether transducer is locked or not
SRF01_Cmd(SRF_ADDRESS, GETSTATUS);                      // Request byte that will tell us if the transducer is locked or unlocked
  while (srf01.available() < 1);
    statusByte = srf01.read();                            // Reads the SRF01 status, The least significant bit tells us if it is locked or unlocked
  int newStatus = statusByte & 0x01;                      // Get status of lease significan bit


//to remove glitch of sensor getting stuck at one range and below or the transducer unlocking for a long period
 unsigned long currentMillis = millis();

  if(newStatus == 0){                                      
    lastlock = millis();
  }
  if(range<30){ 
    lastshortrange = millis();
  }
  if(range>30){ 
    lastlongrange = millis();
  }

if((currentMillis - lastlock > unlocktimeout)||(currentMillis - lastlongrange > sensorglitchtimeout)||(currentMillis - lastshortrange > sensorglitchtimeout)){  
//***RESET ARDUINO FROM SETUP && MAKE BUZZER MAKE NOISE TO ALERT USER***            //as well as letting the user know it is functioning well again, also reminds usser to turn off if it has been left on in the same position
  digitalWrite(Buzzer, HIGH);
  digitalWrite(Buzzer, LOW);
  digitalWrite(Buzzer, HIGH);
  digitalWrite(Buzzer, LOW);  
  digitalWrite(Buzzer, HIGH);
  resetFunc();  //call reset
  delay(100); //delay to ensure the reset but should not get here.
  }

//buzzer beeping function
  if(range<=51){                                                                      // Tells the buzzer to beep when below 51cm
    val=20*(range);                                                                    // Val used for delay time between beep is 20 times that of the distance of the object
    unsigned long currentMillis = millis();
    if((currentMillis - previousMillis > val)&&(BuzzerState==LOW)&&(range>1)){           // if the buzzer is off, the distance is 1cm or above & the calculated pause between beeps has been surpassed
      previousMillis = currentMillis;                                                  // reset milli timer
      BuzzerState = HIGH;                                                                 // change buzzer state from low to high
      }
     else if((currentMillis - previousMillis > interval)&&(BuzzerState==HIGH)&&(range>1)){       // if the buzzer is on, the distance is 1cm or above & the beep has lasted for 50ms
      previousMillis = currentMillis;                                                           // reset milli timer
      BuzzerState = LOW;                                                                        // change buzzer state from High to Low
       }
     else if((range<=1)&&(currentMillis - previousMillis < shortRangeTimeout)){                 // if the distance is below 1cm and has been for less than the shortRangeTimeout
      BuzzerState = HIGH;                                                                      // Set buzzer state high
     }
     else if((range<=1)&&(currentMillis - previousMillis > shortRangeTimeout)){                 // if the distance is below 1cm and has been for more than the shortRangeTimeout
      BuzzerState = LOW;                                                                      // Set buzzer state low
     }
     digitalWrite(Buzzer, BuzzerState);                                                          // turn on/off buzzer depending on the state set at that time. 
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

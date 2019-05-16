// SoftwareSerial - Version: Latest 
#include <SoftwareSerial.h>

/*

*/


#define SRF_TXRX         0x05                                       // Defines pin 5 to be used as RX and TX for SRF01
#define SRF_ADDRESS      0x01                                       // Address of the SFR01
#define GETSOFT          0x5D                                       // Byte to tell SRF01 we wish to read software version
#define GETRANGE         0x54                                       // Byte used to get range from SRF01
#define GETSTATUS        0x5F                                       // Byte used to get the status of the transducer

SoftwareSerial srf01 = SoftwareSerial(SRF_TXRX, SRF_TXRX);      // Sets up software serial port for the SRF01


void setup() {

  srf01.begin(9600);                                      
  Serial.begin(9600);                                   
  srf01.listen();                                         // Make sure that the SRF01 software serial port is listening for data as only one software serial port can listen at a time
  
  delay(200);                                             // Waits some time to make sure everything is powered up
  Serial.println("SRF01 Example");                           
                          
  byte softVer;
  SRF01_Cmd(SRF_ADDRESS, GETSOFT);                        // Request the SRF01 software version
  while (srf01.available() < 1);
    softVer = srf01.read();                               // Read software version from SRF01
  
}

void loop() {
    byte hByte, lByte, statusByte, b1, b2, b3;
  
  SRF01_Cmd(SRF_ADDRESS, GETRANGE);                       // Get the SRF01 to perform a ranging and send the data back to the arduino  
  while (srf01.available() < 2);
  hByte = srf01.read();                                   // Get high byte
  lByte = srf01.read();                                   // Get low byte
  int range = ((hByte<<8)+lByte);                         // Put them together

  Serial.println("Range = ");                                
  Serial.println(range, DEC);                                // Print range result to the screen
  Serial.println("  ");                                      // Print some spaces to the screen to make sure space direcly after the result is clear
  
  SRF01_Cmd(SRF_ADDRESS, GETSTATUS);                      // Request byte that will tell us if the transducer is locked or unlocked
  while (srf01.available() < 1);
    statusByte = srf01.read();                            // Reads the SRF01 status, The least significant bit tells us if it is locked or unlocked
  int newStatus = statusByte & 0x01;                      // Get status of lease significan bit
  if(newStatus == 0){                                      

    Serial.println("Unlocked");                              // Prints the word unlocked followd by a couple of spaces to make sure space after has nothing in
  }
   else {                                      

    Serial.println("Locked   ");                             // Prints the word locked followd by a couple of spaces to make sure that the space after has nothing in
  }

  delay(100);
}

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

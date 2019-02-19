/*  Author:  Brendan Flynn
 Date : last mod = June 20, 2015
 Summary : Arduino driver for FlexVolt Shield.  
 Sends data measured on analog inputs to PC via Serial Comm Port
 
 License:  Apache 2.0 License - http://www.apache.org/licenses/LICENSE-2.0
 
 Physical Inputs : 4 analog inputs, reading raw 10-bit signals from FlexVolt Shield
 Outputs: measured signals, raw or filtered
 
 Note All commands are echoed back.  On computer side, wait for echo to confirm receipt
 */

///////////  Port Mapping 4-Channel Hacker With or Without 2-Channel Stacked//////////////
int chan1 = A3;
int chan2 = A2;
int chan3 = A1;
int chan4 = A0;
int chan5 = A4;
int chan6 = A5;
int analogPins[] = {
  chan1, chan2, chan3, chan4, chan5, chan6}; // array for loops
int tipswitch1 = 5;
int tipswitch2 = 4;
int tipswitch3 = 3;
int tipswitch4 = 2;
int tipswitch5 = 7;
int tipswitch6 = 6;
int tipSwitches[] = {
  tipswitch1, tipswitch2, tipswitch3, tipswitch4, tipswitch5, tipswitch6};
  
///////////  Port Mapping 2-Channel Hacker //////////////
//int chan2 = A4;
//int chan1 = A5;
//int analogPins[] = {
//  chan1, chan2}; // array for loops
//
//int tipswitch2 = 7;
//int tipswitch1 = 6;
//int tipSwitches[] = {
//  tipswitch1, tipswitch2};
   


// constants
const int VERSION = 2;
const int MODELNUMBER = 0;

//  Timing //
// for computer control of sample frequency, just send data every time computer sends 'M'
//
// for Arduino control of sample frequency
// adjustable from computer:
int TimingOffset = -6; // Arduino takes 6 us to execute command - offest for accuracy
const int UserFreqArray[] = {
  1, 10, 50, 100, 200, 300, 400, 500, 1000, 1500, 2000, 4000};
int UserFreqIndex = 8;
int UserFrequency = UserFreqArray[UserFreqIndex];//Hz //modify from computer app
int MaxCustomFreq = 4000; // most signals are in the 100 Hz region.
//  Frequency:  1-2kHz works well.  4kHz pushes timing limits and requires some tweaking.

// Calculate Timing Delay Between Samples:
long measurementdelay = 1000000/long(UserFrequency);// converted to microseconds


// Control Flags
boolean sendflag = false;
boolean dataflag = false;
boolean settingsflag = false;
boolean sendFiltered = false; // flags for data type to send
boolean sendRaw2 = true; // flags for data tye to send


// Control Register
const int REGsize = 9;
unsigned int REG[REGsize];
unsigned int tmpREG[REGsize];
unsigned int REGindex = 0;

// Register variables
int NChan = 8;
int BitDepth = 10;
int DownSampleCount = 0;
int downsampleCounter = 0;
const int DownSampleCountMax = 100;
const int DownSampleCountMin = 0;

// Control Register Masks
//REG0
const unsigned int ChannelMask  = 0b11000000;
const unsigned int FreqIndexMask= 0b00111100;
const unsigned int DataModeMask = 0b00000010;
const unsigned int BitDepthMask = 0b00000001;
//REG1
const unsigned int FilterShiftValMask = 0b00011111;

///////////  Variables  ///////////
long updatetime = 0;  // timer variable

float alpha = 1;
float RC = 0.1;


/* ------------- Plug Testing ---------------- */
const int MaxPlugTestDelay = 100;
int PlugTestDelay = 1;//Hz - check Jack tip switches for plugs
int PlugTestCount = 100;//(UserFrequency*PlugTestDelay)/10; count this high before checking again
int PlugTestCounter = 0; // increments every datasend
int tipswitchcountN = 10;//  1/countN HIGH = true for Arduino digitalRead(2.5V)
int tipswitchCounter = 0;
byte tipswitchvals = 0;
boolean PlugTestFlag = false;


/* ------------- Smoothing Filter ---------------- */
int MaxFilterShift = 30; // 8 is usually a good value
float filter_reg[] = {
  0,0,0,0};
int FilterShiftValdefault = 8;
int FilterShiftVal = FilterShiftValdefault; //10 too high at 1000Hz
// http://www.edn.com/design/systems-design/4320010/A-simple-software-lowpass-filter-suits-embedded-system-applications


void setup(void){
  // set pin modes
  for(int i = 0; i < NChan; i++){ // loop through channels
    pinMode(analogPins[i],INPUT);
    pinMode(tipSwitches[i],INPUT);    
  }
  // establish com
  Serial.begin(230400);  // set baud rate.
}

void loop(){
  if (dataflag){ // Arduino-controlled timing & measurement
    if (micros() > updatetime){  // check if it's time to send next sample
      updatetime = micros()+measurementdelay-TimingOffset;  // reset timer.
      ProcessOutput(); // handles data formats, filters, and channels
    }
  }
  if(PlugTestFlag){
//    PlugTestCounter++;
//    if (PlugTestCounter >= PlugTestCount){
//      TestPlugs();
//      PlugTestCounter = 0;
//    }
  }
}

//  Serial port listener - must handshake before sending data ////////
void serialEvent() {
  if (Serial.available() >= 0) {
    char inChar = Serial.read();
    Serial.write(inChar);
    if (!sendflag){
      if (inChar == 'A') {// computer controlled timing - measure every 'M' from computer
        Serial.write('a');
        return;
      }
      if (inChar == '1') {// computer controlled timing - measure every 'M' from computer
        sendflag = true;
        Serial.write('b');
        return;
      }
      if (inChar == 'X') {// computer controlled timing - measure every 'M' from computer
        DataOff();
        sendflag = false;
        settingsflag = false;
        Serial.write('x');
        return;
      }
      else {
        SendError('s',inChar);
        return;
      }
    }
    else if(sendflag){
      if (settingsflag){
        if (REGindex < REGsize){
          tmpREG[REGindex] = inChar;
          Serial.write(REGindex);
          Serial.write(inChar);
          REGindex++;
          if (REGindex == REGsize){
            Serial.write('y');
          }
          return;
        }
        else if (REGindex >= REGsize){
          settingsflag = false;
          if (inChar != 'Y') { // Quit.  stop.  can still measure and send using computer control (see case 'M')
            DataOff(); // set flag to computer controlled timing
            Serial.write('q');
            return;
          }
          else if(inChar == 'Y'){
            for (int i = 0; i < REGsize; i++){
              REG[i] = tmpREG[i];
            }
            UpdateSettings();
            Serial.write('z');
            return;
          }
        }
      }
      else if (!settingsflag){
        if (inChar == 'M') {// computer controlled timing - measure every 'M' from computer
          ProcessOutput(); // handles data formats, filters, and channels
          DataOff();
          return;
        }
        else if (inChar == 'G'){ // Go.  make with the measuring and sending of data!  Arduino controlled
          Serial.write('g');
          DataOn();
          updatetime = micros()+measurementdelay-TimingOffset; // initialize timer
          return;
        }
        else if (inChar == 'Q') { // Quit.  stop.  can still measure and send using computer control (see case 'M')
          Serial.write('q');
          DataOff();
          return;
        }
        if (inChar == 'X') { // Quit.  stop.  can still measure and send using computer control (see case 'M')
          DataOff(); // set flag to computer controlled timing
          settingsflag = false;
          Serial.write('x');
          return;
        }
        else if (inChar == 'S') { // enter settings menu
          DataOff(); // set Arduino controlled timing off for menu handling
          Serial.write('s');
          settingsflag = true;
          REGindex = 0;
          return;
        }
        else if (inChar == 'V') { // enter settings menu
          DataOff(); // set Arduino controlled timing off for menu handling
          Serial.write('v');
          SendVersionInfo();
          return;
        }
        if (inChar == 'A'){ // always receive upper case
          DataOff();
          Serial.write('a'); // always send back lower case
          sendflag = false;
          return;
        }
        else { // handle all other unknown input values
          SendError('d',inChar);
          DataOff(); // set flag to stop sending data and sort out confusion
          return;
        }
      } // end of if !settingsflag
    } // end of if sendflag
  } // end of if serial available
} // end of function

void DataOff(void){
  dataflag = false;
}

void DataOn(void){
  //long tmpL = 128 << FilterShiftVal;
  float tmpL = 0;
  if (BitDepth == 10){
    tmpL = 512;
  } else if (BitDepth == 8) {
    tmpL = 128;
  }
    
  for (int i = 0; i<NChan; i++){
    filter_reg[i]=tmpL;
  }

  // initialize timing and start counter
  // Now update dependent variables that may have changed
  long measurementdelay = 1000000/long(UserFrequency);// converted to microseconds
  downsampleCounter = 0; // for downsampling
  updatetime = micros()+measurementdelay-TimingOffset;  // reset timer.

  dataflag = true;
}

void send10BitRawVals(void){
  int tmp = 0;
  int val = 0;
  if (NChan == 1){
    Serial.write('H');
  }
  else if (NChan == 2){
    Serial.write('I');
  }
  else if (NChan == 4){
    Serial.write('J'); // highlight next data packet
  }
  else if (NChan == 8){
    Serial.write('K'); // highlight next data packet
  }
  for (int i = 0; i<NChan; i++){ // loop through channels
    if (i < 6){
      val = analogRead(analogPins[i]);  // read channel.  10-bit ADC.  returns int
      val = val << 6;
    } else {val = 512;}
    Serial.write(highByte(val)); // 0000 00**  high byte is mostly 0's
    tmp += (lowByte(val)>>(2*i));
  }
  Serial.write(lowByte(tmp));
}

void send8BitRawVals(void){
  //Serial.write('r');
  int val = 0;
  if (NChan == 1){
    Serial.write('C');
  }
  else if (NChan == 2){
    Serial.write('D');
  }
  else if (NChan == 4){
    Serial.write('E'); // highlight next data packet
  }
  else if (NChan == 8){
    Serial.write('F'); // highlight next data packet
  }
  for (int i = 0; i<NChan; i++){ // loop through channels
    if (i < 6){
      val = analogRead(analogPins[i]);  // read channel.  10-bit ADC.  returns int
      val = val >> 2;
    } else {val = 128;}
    Serial.write(val); // 0000 00**  high byte is mostly 0's
  }
}

void send8BitFilteredVals(void){
  float dt = 1/(float)UserFrequency;
  alpha = dt/ ( RC+dt);
  int val = 0;
  downsampleCounter ++;
  if (downsampleCounter < DownSampleCount){
    //Serial.write('l');
    for (int i = 0; i<NChan; i++){ // loop through channels
      if (i < 6){
        val = analogRead(analogPins[i]);  // read channel.  10-bit ADC.  returns int
        //val = val >> 2;
      } else {val = 512;}
      //filter_reg[i] = filter_reg[i] - (filter_reg[i] >> FilterShiftVal) + long(abs(val-512)+512);
      int tmp2 = val - filter_reg[i];
      filter_reg[i] = filter_reg[i] + alpha*(float)tmp2;
    }
  }
  else {
    //Serial.write('m');
    downsampleCounter = 0;
    if (NChan == 1){
      Serial.write('C');
    }
    else if (NChan == 2){
      Serial.write('D');
    }
    else if (NChan == 4){
      Serial.write('E'); // highlight next data packet
    }
    else if (NChan == 8){
      Serial.write('F'); // highlight next data packet
    }
    for (int i = 0; i<NChan; i++){ // loop through channels
      if (i < 6){
        val = analogRead(analogPins[i]);  // read channel.  10-bit ADC.  returns int
        //val = val >> 2;
      } else {val = 512;}
      //long tmp = filter_reg[i];
      //long tmp2 = tmp >> FilterShiftVal;  // THIS AREA IS BREAKING!!!  PREVENTING DATA SENDING AT EXPECTED RATE
      //filter_reg[i] = tmp - (tmp2) + 128;//long(abs(val-128)+128);
      //filter_reg[i] = filter_reg[i] - (filter_reg[i] >> FilterShiftVal) + long(abs(val-512)+512);
      //val = int(filter_reg[i] >> FilterShiftVal); // FilterShiftVal can be adjusted!
      int tmp2 = filter_reg[i]-val;
      filter_reg[i] = filter_reg[i] + alpha*(float)tmp2;
      val = (int)filter_reg[i];
      val = val >>2;
      Serial.write(val); // 
    }
  }
}

void send10BitFilteredVals(void){
  float dt = 1/(float)UserFrequency;
  alpha = dt/ ( RC+dt);
  int tmp = 0;
  int val = 0;
  downsampleCounter ++;
  if (downsampleCounter < DownSampleCount){
    //Serial.write('l');
    for (int i = 0; i<NChan; i++){ // loop through channels
      if (i < 6){
        val = analogRead(analogPins[i]);  // read channel.  10-bit ADC.  returns int
        //val = val >> 2;
      } else {val = 512;}
      //filter_reg[i] = filter_reg[i] - (filter_reg[i] >> FilterShiftVal) + long(abs(val-512)+512);
      int tmp2 = val-filter_reg[i];
      filter_reg[i] = filter_reg[i] + alpha*(float)tmp2;
    }
  }
  else {
    //Serial.write('m');
    downsampleCounter = 0;
    if (NChan == 1){
      Serial.write('H');
    }
    else if (NChan == 2){
      Serial.write('I');
    }
    else if (NChan == 4){
      Serial.write('J'); // highlight next data packet
    }
    else if (NChan == 8){
      Serial.write('K'); // highlight next data packet
    }
    for (int i = 0; i<NChan; i++){ // loop through channels
      if (i < 6){
        val = analogRead(analogPins[i]);  // read channel.  10-bit ADC.  returns int
        //val = val >> 2;
      } else {val = 512;}
      //long tmp = filter_reg[i];
      //long tmp2 = tmp >> FilterShiftVal;  // THIS AREA IS BREAKING!!!  PREVENTING DATA SENDING AT EXPECTED RATE
      //filter_reg[i] = tmp - (tmp2) + 128;//long(abs(val-128)+128);
      //filter_reg[i] = filter_reg[i] - (filter_reg[i] >> FilterShiftVal) + long(abs(val-512)+512);
      //val = int(filter_reg[i] >> FilterShiftVal); // FilterShiftVal can be adjusted!
      int tmp2 = val-filter_reg[i];
      filter_reg[i] = filter_reg[i] + alpha*(float)tmp2;
      val = (int)filter_reg[i];
      val = val << 6;
      Serial.write(highByte(val)); // 0000 00**  high byte is mostly 0's
      tmp += (lowByte(val)>>(2*i));
    }
    Serial.write(lowByte(tmp));
  }
}

void ProcessOutput(void){
  if (sendRaw2){
    if (BitDepth == 10){
      send10BitRawVals();
    }
    if (BitDepth == 8){
      send8BitRawVals();
    }
  }
  else if(sendFiltered){
    if (BitDepth == 10){
      send10BitFilteredVals();
    }
    if (BitDepth == 8){
      send8BitFilteredVals();
    }
  }
}

void SendVersionInfo(void){
  Serial.write(VERSION);
  Serial.write(0);
  Serial.write(0);
  Serial.write(MODELNUMBER);
}

void UpdateSettings(void){
  /*
 * Control Words
   *
   * REG0 = main/basic user settings
   * REG0<7:6> = Channels, 11=8, 10=4, 01=2, 00=1
   * REG0<5:2> = FreqIndex
   * REG0<1> = DataMode (1 = filtered, 0 = raw)
   * REG0<0> = Data bit depth.  1=10bits, 0 = 8bits
   *
   * REG1 = Filter Shift Val + Prescalar Settings
   * REG1<4:0> = filter shift val, 0:31, 5-bits
   * REG1<7:5> = PS setting.
   *              000 = 2
   *              001 = 4
   *              010 = 8
   *              011 = 16 // not likely to be used
   *              100 = 32 // not likely to be used
   *              101 = 64 // not likely to be used
   *              110 = 128// not likely to be used
   *              111 = off (just use 48MHz/4)
   *
   * REG2 = Manual Frequency, low byte (16 bits total)
   * REG3 = Manual Frequency, high byte (16 bits total)
   *
   * REG4 = Time adjust val (8bits, -6:249)
   *
   * REG5 & REG6 Timer Adjustment
   * (add Time Adjust to x out of N total counts to 250)
   * REG5<7:0> = partial counter val, low byte, 16 bits total
   * REG6<7:0> = partial counter val, high byte, 16 bits total
   *
   * REG7<7:0> = down sampling value (mainly for smoothed data)
   *
   * REG8<7:0> = Plug Test Frequency
   */
  int tmp = 0;
  int tmp2 = 0;

  //REG0
  tmp = REG[0]&ChannelMask;
  Serial.write(0);
  Serial.write(tmp);
  Serial.write(0);
  tmp = tmp>>6;
  Serial.write(1);
  Serial.write(tmp);
  Serial.write(1);
  if (tmp == 3){NChan = 8;}
  if (tmp == 2){NChan = 4;}
  if (tmp == 1){NChan = 2;}
  if (tmp == 0){NChan = 1;}
  Serial.write(2);
  Serial.write(NChan);
  Serial.write(2);

  tmp = REG[0]&FreqIndexMask;
  tmp = tmp>>2;
  if (tmp >= 0 && tmp < sizeof(UserFreqArray)){ // handle out of bounds
    UserFreqIndex = tmp;
    UserFrequency = UserFreqArray[UserFreqIndex]; // store new frequency
    measurementdelay = 1000000/long(UserFrequency);
  }
  else {
    SendError('I',tmp);
  }

  tmp = REG[0]&DataModeMask;
  if(tmp>0){
    sendRaw2 = false;
    sendFiltered = true;
    Serial.write('f');
  }
  else if(tmp == 0){
    sendFiltered = false;
    sendRaw2 = true;
    Serial.write('r');
  }

  tmp = REG[0]&BitDepthMask;
  if(tmp == 0){
    BitDepth = 8;
  }
  else if(tmp > 0){
    BitDepth = 10;
  }

  // REG1
  tmp = REG[1]&FilterShiftValMask;
  if (tmp >= 0 && tmp <= MaxFilterShift){
    FilterShiftVal = tmp;
  }
  else{
    SendError('f',tmp);
  }

  //    tmp = REG[1]; // for USB/Bluetooth versions

  // REG2,3  Custom Frequency.  Overwrites If UserFrequency is > 0
  tmp = REG[3];
  tmp = tmp<<8;
  tmp2 = REG[2];
  tmp += tmp2;
  if (tmp > 0 && tmp < MaxCustomFreq){
    UserFrequency = tmp;
  }

  // REG4 Timer0 Adjust Value
  TimingOffset = REG[4]-6;//shift it with the -6

    // REG5, 6 Timer0 Adjust Value Partial count implementation
  //    tmp = REG[6];
  //    tmp = tmp << 8;
  //    tmp2 = REG[5];
  //    Timer0PartialCounter = tmp + tmp2;

  // REG 7 Downsampling.  Transmit a lower datarate, especially for smoothed signal
  tmp = REG[7];
  if (tmp >= DownSampleCountMin && tmp <= DownSampleCountMax){
    DownSampleCount = tmp;
  }
  downsampleCounter = 0;
  Serial.write(DownSampleCount);

  // REG8 PlugTest.  Delay = number of seconds.  if 0, no plugtest
  tmp = REG[8];
  if (tmp == 0){
    PlugTestFlag = false;
    Serial.write('z');
  }
  if (tmp > 0 && tmp < MaxPlugTestDelay){
    Serial.write('w');
    PlugTestFlag = true;
    PlugTestDelay = tmp;
    PlugTestCount = (UserFrequency*PlugTestDelay)/10; // note the divide by 10!!
    if (PlugTestCount <= 0){
      PlugTestCount = 1;
    }
  }
}

void TestPlugs(void){
  byte mask = 00000001;
  for(int i = 0; i < NChan; i++){ // loop through channels
    if(digitalRead(tipSwitches[i]) == HIGH){
      tipswitchvals = tipswitchvals | mask;
      mask = mask << 1;
    }
  }
  tipswitchCounter++;
  if (tipswitchCounter >= tipswitchcountN){
    Serial.write('p');
    Serial.write(tipswitchvals);
    tipswitchvals = 0;
    tipswitchCounter = 0;
  }
}

void SendError(char code, char msg){
  Serial.write('e');
  Serial.write(code);
  Serial.write(msg);
}


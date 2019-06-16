/*
  Based on the SendDemo example from the RC Switch library
  https://github.com/sui77/rc-switch/
*/

#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  
  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(10);

  // Optional set pulse length.
  mySwitch.setPulseLength(394);
  
  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(1);
  
  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);
}

const int MaxChars = 24;
char strValue[MaxChars+1];
int index = 0;

void loop() {
  // Binary code - button 3
//  mySwitch.send("111111000110010000000001");
//  delay(250);
  if (Serial.available())
  {
    char ch = Serial.read();
    if (index < MaxChars){
      strValue[index++] = ch; //add ascii to the string
    }
    else
    {
      //if buffer is full
      strValue[index] = 0;
      index=0;
      mySwitch.send(strValue);
    }
  }
//Serial.println(strValue);
    
//    String ss = Serial.readString();
//    unsigned char ss = "111111000110010000000001";
//    mySwitch.send(ss);
//    delay(250);
//    Serial.println(ss);
}

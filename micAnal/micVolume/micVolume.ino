/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/
 
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
 
void setup() 
{

  pinMode(6, OUTPUT); // our digital pin7 is an output
  
   Serial.begin(9600);
   // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(10);

  // Optional set pulse length.
  mySwitch.setPulseLength(394);
  
  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(1);
}


void blink(){
//  mySwitch.send("111111000110010000000001");
//  delay(50);
//  mySwitch.send("111111000110010000000001");

//  analogWrite(6, 50);
//  digitalWrite(7, HIGH);
  
  
}
 
void loop() 
{
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
 
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * 5.0) / 1024;  // convert to volts
 
//   Serial.println(volts);

  if (volts > 2.8) {
//    blink();
  }
  if (volts < 1) { volts = 0; }
  analogWrite(6, 255-volts*52);
  Serial.println(255-volts*52);
}

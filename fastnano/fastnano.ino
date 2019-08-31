/* Fast Hartley Transform for responsive lighting
 * Anthony Bennett
 * With code from Andrew Tuline
 * 
 */

#define qsubd(x, b) ((x>b)?b:0)                               // A digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b) ((x>b)?x-b:0)                             // Unsigned subtraction macro. if result <0, then => 0.

// FHT Definitions
#define LOG_OUT 1                                             // Use logarithmic based bins (is required for the library to run).
//#define LIN_OUT 1 //linear output
#define FHT_N 256                                             // Set to 256 point fht. Any less, and the upper ranges won't work well.
#define DC_OFFSET  0                                        // DC offset in mic signal. Should probably be about 512.
#define MIC_PIN 0                                             // We're using A0.

#include <FHT.h>                                              // FHT library at http://wiki.openmusiclabs.com/wiki/ArduinoFHT

#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 3
CRGB leds[NUM_LEDS];

//set LED pins
#define RED 4
#define GRN 5
#define BLU 6


void setup() {  
  Serial.begin(57600);                                        // Initialize serial port for debugging.
  delay(100);                                                // Soft startup to ease the flow of electrons.

  LEDS.addLeds<WS2812,DATA_PIN,GRB>(leds,NUM_LEDS);
  LEDS.setBrightness(84);

  pinMode(12, OUTPUT); //Microphone Ground
  pinMode(11, OUTPUT); //Microphone Power
  digitalWrite(12, LOW);
  digitalWrite(11, HIGH); //Powering that shit without wires
  
  pinMode(RED, OUTPUT); 
  pinMode(GRN, OUTPUT); 
  pinMode(BLU, OUTPUT); // our digital pins 2 3 4 are R G B respectively

// Setup the ADC for polled 10 bit sampling on analog pin 0 at 19.2kHz.
  cli();                                  // Disable interrupts.
  ADCSRA = 0;                             // Clear this register.
  ADCSRB = 0;                             // Ditto.
  ADMUX = 0;                              // Ditto.
  ADMUX |= (MIC_PIN & 0x07);              // Set A0 analog input pin.
//  ADMUX |= (1 << REFS0);                  // Set reference voltage  (analog reference(external), or using 3.3V microphone on 5V Arduino.
                                          // Set that to 1 if using 5V microphone or 3.3V Arduino.
//  ADMUX |= (1 << ADLAR);                  // Left justify to get 8 bits of data.                                          
  ADMUX |= (0 << ADLAR);                  // Right justify to get full 10 A/D bits.

//  ADCSRA |= bit (ADPS0) | bit (ADPS2);                //  32 scaling or 38.5 KHz sampling
  ADCSRA |= bit (ADPS1) | bit (ADPS2);                //  Set ADC clock with 64 prescaler where 16mHz/64=250kHz and 250khz/13 instruction cycles = 19.2khz sampling.
//  ADCSRA |= bit (ADPS0) | bit (ADPS1) | bit (ADPS2);    // 128 prescaler with 9.6 KHz sampling
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1);  // Set ADC clock with 64 prescaler where 16mHz/64=250kHz and 250khz/13 instruction cycles = 19.2khz sampling.
// ADCSRA |= (1 << ADPS2) | (1 << ADPS0);   // Set ADC clock with 32 prescaler for 38.5 KHz sampling.
  ADCSRA |= (1 << ADATE);                 // Enable auto trigger.
//  ADCSRA |= (1 << ADIE);                  // Enable interrupts when measurement complete (if using ISR method). Sorry, we're using polling here.
  ADCSRA |= (1 << ADEN);                  // Enable ADC.
  ADCSRA |= (1 << ADSC);                  // Start ADC measurements.
  sei();                                  // Re-enable interrupts.

  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  TIMSK0 = 0; // turn off timer0 for lower jitter
  

} // setup()

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

void loop() {
//  showfps();                                                  // Debug output of how many frames per second we're getting. Comment this out in production.
  getFHT();                                                   // Let's take FHT_N samples and crunch 'em.
  fhtDisplay();                                               // Let's calculate the LED display from our FHT output array.

  
}  // loop()



void getFHT() {
  get_sound();                                                // High speed sound sampling.
  fht_window();                                               // Window the data for better frequency response.
  fht_reorder();                                              // Reorder the data before doing the fht.
  fht_run();                                                  // Process the data in the fht.
  fht_mag_log();                                              // I guess we'll be converting to logarithm.
//  fht_mag_lin();                                            // Linear conversion if needed
} // GetFHT()



void get_sound() {                                            // Uses high speed polled analog sampling and NOT analogRead().
// Here's the slow and jittery 8KHz method of sampling sound, which we no longer use.
//  for (int i = 0 ; i < FHT_N ; i++) fht_input[i] = analogRead(inputPin) - DC_OFFSET;
  
    cli();
    for (int i = 0 ; i < FHT_N ; i++) {                       // Save 256 samples. No more, no less.
      while(!(ADCSRA & 0x10));                                // Wait for adc to be ready.
      ADCSRA = 0xf5; // restart adc
      fht_input[i] = ADC - DC_OFFSET;                         // Get the full 10 bit A/D conversion and center it.
    
//      Serial.print(fht_input[40]);                              // Serial plot graph of our sampling.
//      Serial.print(" ");                                      // Lowest and highest values are graphed so that the plot isn't auto-scaled.
//      Serial.print(0);                                        // Lowest value
//      Serial.print(" ");
//      Serial.print(512);                                      // Highest value
//      Serial.println(" ");
    }
    sei();
} // get_sound()


//bin and threshold tuning
  #define redCh 2
  #define grnCh 21
  #define bluCh 22
  #define thresh 53

void fhtDisplay() {
//  int N = sizeof fht_log_out;

  //debug
  for (int i = 0; i < sizeof fht_log_out/2; i++) { //max 256
  Serial.print(fht_log_out[i]);
//  Serial.print(fht_lin_out[i]);
  Serial.print(" ");
  }
  Serial.println("");  

  
  //set to 0 if below threshold
  if (fht_log_out[redCh] < thresh) { fht_log_out[redCh] = 0; }
  if (fht_log_out[bluCh] < thresh) { fht_log_out[bluCh] = 0; }
  if (fht_log_out[grnCh] < thresh) { fht_log_out[grnCh] = 0; }
  //write PWM red
//  analogWrite(RED, fht_log_out[redCh]);
  //write PWM green
//  analogWrite(GRN, 255-fht_log_out[grnCh]*2);
  //write PWM blue
//  analogWrite(BLU, 255-fht_log_out[bluCh]*2);
    static uint8_t hue = 200;
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(0, 0, 0);
  }
    int ledlen = (fht_log_out[redCh]*2.2)/10;
//    Serial.print(fht_log_out[redCh]);
//    Serial.print(" ");
//    Serial.print(ledlen);
//    Serial.println("");
  for(int i = 0; i < ledlen; i++) {
    // Set the i'th led to red 
//    leds[i] = CHSV(hue++, 255, fht_log_out[redCh]*1.5);
      leds[i] = CHSV(hue, 255, 255);
    // Show the leds
  }
  FastLED.show(); 


//  Serial.print(fht_log_out[redCh]);
//  Serial.print(" ");
//  Serial.print(fht_log_out[grnCh]);
//  Serial.print(" ");
//  Serial.println(fht_log_out[bluCh]);
  
} // fhtDisplay()



void showfps() {

  long currentMillis = 0;
  static long lastMillis = 0;
  static long loops = 0;
  
  currentMillis=millis();                                     // Determine frames per second
  loops++;
  if(currentMillis - lastMillis > 1000){
    Serial.println(loops);
    lastMillis = currentMillis;
    loops = 0;
  }
} // showfps()

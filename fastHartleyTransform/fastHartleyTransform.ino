#define LOG_OUT 1 // use the log output function
#define FHT_N 256 // set to 256 point fht

#include <FHT.h> // include the library

//macros
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

//modes
//#define Use3.3 // use 3.3 voltage. the 5v voltage from usb is not regulated. this is much more stable.
//#define ADCReClock // switch to higher clock, not needed if we are ok with freq between 0 and 4Khz.
#define ADCFlow // read data from adc with free-run (not interupt). much better data, dc low. hardcoded for A0.

#define FreqLog // use log scale for FHT frequencies
#ifdef FreqLog
#define FreqOutData fht_log_out
#define FreqGainFactorBits 0
#else
#define FreqOutData fht_lin_out8
#define FreqGainFactorBits 3
#endif
#define FreqSerialBinary

#define VolumeGainFactorBits 0

void setup() {
  Serial.begin(9600); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

  pinMode(6, OUTPUT); // our digital pin7 is an output
  pinMode(5, OUTPUT); // our digital pin7 is an output
  pinMode(9, OUTPUT); // our digital pin7 is an output
  pinMode(12, OUTPUT); //12 for mic power
}

void MeasureFHT()
{
    long t0 = micros();
    for (int i = 0; i < FHT_N; i++) { // save 256 samples
        while (!(ADCSRA & /*0x10*/_BV(ADIF))); // wait for adc to be ready (ADIF)
        sbi(ADCSRA, ADIF); // restart adc
        byte m = ADCL; // fetch adc data
        byte j = ADCH;
        int k = ((int)j << 8) | m; // form into an int
        k -= 0x0200; // form into a signed int
        k <<= 6; // form into a 16b signed int
        fht_input[i] = k; // put real data into bins
    }
    long dt = micros() - t0;
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log();
 
    // print as text
//    for (int i = 2; i < 20 / 2; i++)
////    for (int i=0;i<2;i++)
//    {
//        Serial.print(FreqOutData[i]);
//        Serial.print(',');
//    }
//    Serial.println(' ');

    if (FreqOutData[2] < 140) { FreqOutData[2] = 0; }
    if (FreqOutData[3] < 140) { FreqOutData[3] = 0; }
    if (FreqOutData[4] < 140) { FreqOutData[4] = 0; }
    analogWrite(6, 255-FreqOutData[2]*2);
//    Serial.println(255-FreqOutData[2]);
    analogWrite(5, 255-FreqOutData[3]*2);
    analogWrite(9, 255-FreqOutData[4]*2);

//    Serial.print(FreqOutData[0]);
//    Serial.print(',');
//    Serial.print(FreqOutData[2]);
//    Serial.println(',');
//    long sample_rate = FHT_N * 1000000l / dt;
//    Serial.println(dt);
//    Serial.print(',');
//    Serial.println(sample_rate);
}

void loop() {
  while(1) { // reduces jitter
    MeasureFHT();
//    delay(50);
  }
}

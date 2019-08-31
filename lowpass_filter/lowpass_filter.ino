#include <Filters.h>

void setup() {
  // filters out changes faster that 5 Hz.
  Serial.begin(9600);
}

//Low pass bessel filter order=1 alpha1=0.02 
class  FilterBeLp1
{
  public:
    FilterBeLp1()
    {
      v[0]=0.0;
    }
  private:
    float v[2];
  public:
    float step(float x) //class II 
    {
      v[0] = v[1];
      v[1] = (5.919070381840546569e-2 * x)
         + (0.88161859236318906863 * v[0]);
      return 
         (v[0] + v[1]);
    }
};

void loop() {
  while( true ) {
    
  // create a one pole (RC) lowpass filter
//  float filterFrequency = 100.0;
//    FilterOnePole lowpassFilter( LOWPASS, filterFrequency );   
//    lowpassFilter.input( analogRead( A0 ) );
    int test = analogRead(A0);
    FilterBeLp1();
    Serial.println(test);
  }
}

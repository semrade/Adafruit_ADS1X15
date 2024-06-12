#include <iostream>
#include "I2c.h"
#include "Adafruit_ADS1X15.h"

using namespace std;

int main (int argc, char *argv[])
{
    Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
    try 
    {
        ads.begin();
        usleep(100);

        cout<<"Getting single-ended readings from AIN0..3"<<endl;
        cout<<"ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)"<<endl;
        
        // Start continuous conversions.
        ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, /*continuous=*/true);
        
        int16_t results[4] = {0};
        int16_t adc0, adc1, adc2, adc3;
        float volts0, volts1, volts2, volts3;
        while (1)
        {
            /*
            if(!ads.conversionComplete())
            {
                continue;
            }
            */

            adc0 = ads.readADC_SingleEnded(0);
            adc1 = ads.readADC_SingleEnded(1);
            adc2 = ads.readADC_SingleEnded(2);
            adc3 = ads.readADC_SingleEnded(3);

            volts0 = ads.computeVolts(adc0);
            volts1 = ads.computeVolts(adc1);
            volts2 = ads.computeVolts(adc2);
            volts3 = ads.computeVolts(adc3);

            cout<<"-----------------------------------------------------------"<<endl;
            cout<<"AIN0: "<<adc0<<"  "<<volts0<<"V"<<endl;
            cout<<"AIN1: "<<adc1<<"  "<<volts1<<"V"<<endl;
            cout<<"AIN2: "<<adc2<<"  "<<volts2<<"V"<<endl;
            cout<<"AIN3: "<<adc3<<"  "<<volts3<<"V"<<endl;

            sleep(1);
        }
        
    } 
    catch (const std::runtime_error &e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        ads.end();
    }
    return 0;
}
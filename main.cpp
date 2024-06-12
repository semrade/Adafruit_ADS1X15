#include <iostream>
#include <string>
#include <stdexcept> // For std::invalid_argument and std::out_of_range
#include "I2c.h"
#include "Adafruit_ADS1X15.h"

using namespace std;

// Function declarations for each mode
void mode1(Adafruit_ADS1115 &ads) {
    std::cout << "Mode 1 selected: Performing conversion 1." << std::endl;
    // Add your conversion logic for mode 1 here
    // Example: throw std::runtime_error("Mode 1 error");

    if (!ads.begin()) {
        std::cout<<"Failed to initialize ADS."<<std::endl;
    while (1);
    }

    cout<<"Getting single-ended readings from AIN0..3"<<endl;
    cout<<"ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)"<<endl;
    
    // Start continuous conversions.
    //ads.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, /*continuous=*/true);
    

    int16_t adc0, adc1, adc2, adc3;
    float volts0, volts1, volts2, volts3;
    while (1)
    {

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

        usleep(1000);
    }
        
}

void mode2(Adafruit_ADS1115 &ads) {
    
    // Add your conversion logic for mode 2 here
    // Example: throw std::runtime_error("Mode 2 error");
    std::cout << "Mode 2 selected: Performing conversion 2." << std::endl;
    std::cout << "Single-ended readings from AIN0 with >3.0V comparator"<<std::endl;
    std::cout << "ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)"<<std::endl;
    std::cout << "Comparator Threshold: 1000 (3.000V)"<<std::endl;

    if (!ads.begin()) {
        std::cout<<"Failed to initialize ADS."<<std::endl;
    while (1);
    }
    // Setup 3V comparator on channel 0
    ads.startComparator_SingleEnded(0, 1000);
    int16_t adc0;
    while(1)
    {
        // Comparator will only de-assert after a read
        adc0 = ads.getLastConversionResults();
        std::cout<<"AIN0: "<<adc0<<std::endl;

        usleep(1000);
    }

}

void mode3(Adafruit_ADS1115 &ads) 
{
    std::cout << "Mode 3 selected: Performing conversion 3." << std::endl;
    // Add your conversion logic for mode 3 here
    // Example: throw std::runtime_error("Mode 3 error");
    std::cout << "Getting differential reading from AIN0 (P) and AIN1 (N)"<<std::endl;
    std::cout << "ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)"<<std::endl;

    // The ADC input range (or gain) can be changed via the following
    // functions, but be careful never to exceed VDD +0.3V max, or to
    // exceed the upper and lower limits if you adjust the input range!
    // Setting these values incorrectly may destroy your ADC!
    //                                                                ADS1015  ADS1115
    //                                                                -------  -------
    // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
    // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
    // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
    // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
    // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
    // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

    if (!ads.begin()) {
        std::cout<<"Failed to initialize ADS."<<std::endl;
        while (1);
    }

    int16_t results;
    /* Be sure to update this value based on the IC and the gain settings! */
    float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
    //float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */
    
    while(1)
    {

    results = ads.readADC_Differential_0_1();

    std::cout<<"Differential: "; 
    std::cout<<results; 
    std::cout<<"("; 
    std::cout<<results * multiplier; 
    std::cout<<"mV)"<<std::endl;

    usleep(10000);
    }
}

int main(int argc, char* argv[]) {

    try {
        if (argc != 2) {
            throw std::invalid_argument("Usage: " + std::string(argv[0]) + " <mode>\nModes: 1, 2, 3");
        }

        int mode = std::stoi(argv[1]);
        Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

        switch (mode) {
            case 1:
                try {
                    mode1(ads);
                } catch (const std::exception& e) {
                    std::cerr << "Error in mode 1: " << e.what() << std::endl;
                    return 1;
                }
                break;
            case 2:
                try {
                    mode2(ads);
                } catch (const std::exception& e) {
                    std::cerr << "Error in mode 2: " << e.what() << std::endl;
                    return 1;
                }
                break;
            case 3:
                try {
                    mode3(ads);
                } catch (const std::exception& e) {
                    std::cerr << "Error in mode 3: " << e.what() << std::endl;
                    return 1;
                }
                break;
            default:
                throw std::invalid_argument("Invalid mode selected. Please choose 1, 2, or 3.");
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Mode value is out of range." << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

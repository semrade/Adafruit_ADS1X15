#include <iostream>
#include <string>
#include <stdexcept> // For std::invalid_argument and std::out_of_range
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <mutex>
#include <cstring>
#include "I2c.h"
#include "Adafruit_ADS1X15.h"
#include "driver.h"

using namespace std;

#define NUM_READINGS 4
#define BUFFER_SIZE 22
//#define TELE 

void update_display_int(char *format, int value);
void update_display_double(char *format, double value);
void display_values(const char* message);
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
    

    int16_t adc[NUM_READINGS]={0};
    float volts[NUM_READINGS]={0.0};
    while (1)
    {

        adc[0] = ads.readADC_SingleEnded(0);
        adc[1] = ads.readADC_SingleEnded(1);
        adc[2] = ads.readADC_SingleEnded(2);
        adc[3] = ads.readADC_SingleEnded(3);

        volts[0] = ads.computeVolts(adc[0]);
        volts[1] = ads.computeVolts(adc[1]);
        volts[2] = ads.computeVolts(adc[2]);
        volts[3] = ads.computeVolts(adc[3]);

        cout<<"-----------------------------------------------------------"<<endl;
        cout<<"AIN0: "<<adc[0]<<"  "<<volts[0]<<"V"<<endl;
        cout<<"AIN1: "<<adc[1]<<"  "<<volts[1]<<"V"<<endl;
        cout<<"AIN2: "<<adc[2]<<"  "<<volts[2]<<"V"<<endl;
        cout<<"AIN3: "<<adc[3]<<"  "<<volts[3]<<"V"<<endl;

        usleep(10000);
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
void mode3(Adafruit_ADS1115 &ads) {
    std::cout << "Mode 3 selected: Performing conversion 3." << std::endl;
    // Add your conversion logic for mode 3 here
    // Example: throw std::runtime_error("Mode 3 error");
    std::cout << "Getting differential reading from AIN0 (P) and AIN1 (N)"<<std::endl;
    std::cout << "ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)"<<std::endl;

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

    usleep(1000);
    }
}
void parentProcess(int pipe, Adafruit_ADS1115 &ads)
{
    int16_t adc[NUM_READINGS]={0};
    float volts[NUM_READINGS]={0};
#ifdef TELE
    auto start = std::chrono::high_resolution_clock::now();    
#endif
    adc[0] = ads.readADC_SingleEnded(0);
    adc[1] = ads.readADC_SingleEnded(1);
    adc[2] = ads.readADC_SingleEnded(2);
    adc[3] = ads.readADC_SingleEnded(3);

    volts[0] = ads.computeVolts(adc[0]);
    volts[1] = ads.computeVolts(adc[1]);
    volts[2] = ads.computeVolts(adc[2]);
    volts[3] = ads.computeVolts(adc[3]);

    //usleep(100);

    // Read ADC and send data to child
    // Envoyer les valeurs ADC au processus enfant
    if (write(pipe, adc, sizeof(int16_t) * NUM_READINGS) == -1)
    {
        std::cerr << "Erreur d'écriture dans le pipe pour les valeurs raw ADC: " << strerror(errno) << std::endl;
    }
    if (write(pipe, volts, sizeof(float) * NUM_READINGS) == -1)
    {
        std::cerr << "Erreur d'écriture dans le pipe pour les valeurs volt ADC : " << strerror(errno) << std::endl;
    }

#ifdef TELE
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Parent process time: " << elapsed.count() << " ms" << std::endl;
#endif
}
void childProcess(int pipe)
{
    int16_t raw_received_values[NUM_READINGS]={0};
    float volt_received_values[NUM_READINGS]={0};
    static int ctn = 0;
#ifdef TELE    
auto start = std::chrono::high_resolution_clock::now();   
#endif
    cout<<"-----------------------------------------------------------"<<endl;
    cout<<"Getting single-ended readings from AIN0..3"<<endl;
    cout<<"ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)"<<endl;
    
    if(read(pipe, raw_received_values, sizeof(int16_t) * NUM_READINGS) == -1)
    {
        std::cerr << "Erreur de lecture du pipe pour les valeurs raw ADC: " << strerror(errno) << std::endl;
    }
    // Lire les valeurs ADC du pipe
    if(read(pipe, volt_received_values, sizeof(float) * NUM_READINGS) == -1)
    {
        std::cerr << "Erreur de lecture du pipe pour les valeurs volt ADC: " << strerror(errno) << std::endl;
    }
    
    
    // Affichage des valeurs
    cout<<"-----------------------------------------------------------"<<endl;
    cout<<"AIN0: "<<raw_received_values[0]<<"  "<<volt_received_values[0]<<"V"<<endl;
    cout<<"AIN1: "<<raw_received_values[1]<<"  "<<volt_received_values[1]<<"V"<<endl;
    cout<<"AIN2: "<<raw_received_values[2]<<"  "<<volt_received_values[2]<<"V"<<endl;
    cout<<"AIN3: "<<raw_received_values[3]<<"  "<<volt_received_values[3]<<"V"<<endl;

        if (ctn < 10) {
            // Affichage des valeurs brutes
            // Clear SSD
            //SSD1306_Fill(0x00);
            SSD1306_SetCursor(2, 0);
            display_values("ADS1115 raw values\n");

            update_display_int("AIN0: : %d LSB", raw_received_values[0]);
            update_display_int("AIN1: : %d LSB", raw_received_values[1]);
            update_display_int("AIN2: : %d LSB", raw_received_values[2]);
            update_display_int("AIN3: : %d LSB", raw_received_values[3]);
        } else if (ctn < 20) {
            // Affichage des valeurs physiques
            // Clear the LCD
            //SSD1306_Fill(0x00);
            SSD1306_SetCursor(2, 0);
            display_values("ADS1115 Phys values\n");

            // Afficher les différentes valeurs
            update_display_double("AIN0: : %.2f V", volt_received_values[0]);
            update_display_double("AIN1: : %.2f V", volt_received_values[1]);
            update_display_double("AIN2: : %.2f V", volt_received_values[2]);
            update_display_double("AIN3: : %.2f V", volt_received_values[3]);
        }

        ctn++;
        if (ctn >= 20) {
            ctn = 0;  // Réinitialiser le compteur après 2000 cycles
        }


#ifdef TELE
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout <<  "Child process time: " << elapsed.count() << " ms" << std::endl;
#endif
}
int main(int argc, char* argv[]) {
    /******************** pour rendre le process plus prioritaire **********************/
    struct sched_param param;
    param.sched_priority = 99; // Priorité entre 1 (basse) et 99 (élevée)

    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        std::cerr << "Erreur lors de la définition de l'ordonnancement en temps réel: " << strerror(errno) << std::endl;
        return 1;
    }
    /**********************************************************************************/

    try {
        if (argc != 2) {
            throw std::invalid_argument("Usage: " + std::string(argv[0]) + " <mode>\nModes: 1, 2, 3 4");
        }

        int mode = std::stoi(argv[1]);
        // Create a shared mutex in memory
        pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
        if (mutex == MAP_FAILED) {
            throw std::invalid_argument("mmap");
            exit(1);
        }

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(mutex, &attr);

        // Create a pipe
        int pipeFd[2];
        if (pipe(pipeFd) == -1) {
            throw std::invalid_argument("pipe");
            exit(1);
        }

        // Creat the ADS object
        Adafruit_ADS1115 ads; 

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
            case 4:
                try {
                    if (!ads.begin()) 
                    {
                        std::cerr<<"Failed to initialize ADS."<<std::endl;
                        while (1);
                    }
                    pid_t pid = fork();
                    if (pid > 0) 
                    {
                        // Parent process
                        close(pipeFd[0]); // Close reading end
                        while (true) {
                            pthread_mutex_lock(mutex);
                            parentProcess(pipeFd[1], ads);
                            pthread_mutex_unlock(mutex);
                            usleep(100000); // 100ms for data acquisition
                        }
                        close(pipeFd[1]); // Close writing end
                        wait(nullptr);
                    } else {
                        // Child process
                        close(pipeFd[1]); // Close writing end
                        SSD1306_DisplayInit();
                        SSD1306_SetCursor(0, 0);
                        SSD1306_Fill(0x00);
                        //SSD1306_String(reinterpret_cast<unsigned char*>("ADS1115 disp Values\n"));
                        display_values("ADS1115 disp Values\n");
                        while (true) {
                            pthread_mutex_lock(mutex);
                            childProcess(pipeFd[0]);
                            pthread_mutex_unlock(mutex);
                            usleep(100000); // 100ms for data display
                        }
                        close(pipeFd[0]); // Close reading end
                }
                    
                } catch (const std::exception& e) {
                    std::cerr << "Error in mode 4: " << e.what() << std::endl;
                    return 1;
                }

                break;
            default:
                throw std::invalid_argument("Invalid mode selected. Please choose 1, 2, or 3.");
        }

        pthread_mutex_destroy(mutex);
        munmap(mutex, sizeof(pthread_mutex_t));

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


void update_display_int(char *format, int value) {
    char buffer[BUFFER_SIZE];

    // Initialiser le buffer avec des espaces
    memset(buffer, ' ', BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0';  // Ajouter le caractère nul de fin

    // Formater le texte à afficher dans un buffer temporaire
    char temp_buffer[BUFFER_SIZE];
    int length = snprintf(temp_buffer, BUFFER_SIZE, format, value);

    // Si la chaîne formatée dépasse la taille du buffer, la tronquer
    if (length >= BUFFER_SIZE) {
        length = BUFFER_SIZE - 1;
    }

    // Copier la chaîne formatée dans le buffer principal
    memcpy(buffer, temp_buffer, length);

    // Afficher le texte sur l'écran
    display_values(buffer);
}


void update_display_double(char *format, double value) {
    char buffer[BUFFER_SIZE];

    // Initialiser le buffer avec des espaces
    memset(buffer, ' ', BUFFER_SIZE - 1);
    buffer[BUFFER_SIZE - 1] = '\0';  // Ajouter le caractère nul de fin

    // Formater le texte à afficher dans un buffer temporaire
    char temp_buffer[BUFFER_SIZE];
    int length = snprintf(temp_buffer, BUFFER_SIZE, format, value);

    // Si la chaîne formatée dépasse la taille du buffer, la tronquer
    if (length >= BUFFER_SIZE) {
        length = BUFFER_SIZE - 1;
    }

    // Copier la chaîne formatée dans le buffer principal
    memcpy(buffer, temp_buffer, length);

    // Afficher le texte sur l'écran
    display_values(buffer);
}

void display_values(const char* message) {
    //const char* message = "ADS1115 disp Values\n";
    unsigned char buffer[50]; // Assurez-vous que la taille est suffisante
    std::strcpy(reinterpret_cast<char*>(buffer), message);
    SSD1306_String(buffer);
}
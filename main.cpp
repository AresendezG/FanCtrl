#include <wiringPi.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <fstream>
#include <csignal>
#include <cmath>

/*
* Run gpio readall first to make sure WiringPi is working
* Export your GPIO pins so you can use them without sudo
*  Define your pins based on the BCM_GPIO number
*/

// Pin number definition
#define	POWERLED	18
#define FAN_CTRL    22
#define ENABLE_PWM  23

// Function to wait for N minutes
void waitMins(int minQty)
{
    using std::chrono::system_clock;
    std::time_t tt = system_clock::to_time_t(system_clock::now());
    struct std::tm* ptm = std::localtime(&tt);
    ptm->tm_min += minQty; ptm->tm_sec = 0;
    std::this_thread::sleep_until(system_clock::from_time_t(mktime(ptm)));
}

// Function to wait for N seconds
void waitSecs(int Secs)
{
    using std::chrono::system_clock;
    std::time_t tt = system_clock::to_time_t(system_clock::now());
    struct std::tm* ptm = std::localtime(&tt);
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

void ToggleStatus(int PinNumber) {
    
        bool currentStatus = digitalRead(PinNumber);
        bool newStatus = !(currentStatus);
        digitalWrite(PinNumber, newStatus);
    
}


double GetTempValue() {

    FILE* temperatureFile;
    double T;
    temperatureFile = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (temperatureFile == NULL)
        return -150;
    fscanf(temperatureFile, "%lf", &T);
    T /= 1000;
    // printf("The temperature is %6.3f C.\n", T);
    fclose(temperatureFile);
    return T;

}

void GetInitConfigs(int* SettingsArray) {

    FILE* settingsFile;

    settingsFile = fopen("/etc/fan_ctrl/fan_settings", "r");
    if (settingsFile == NULL)
    {
        // Error Reading the Settings File
        SettingsArray[0] = 1; // Interval Wait
        SettingsArray[1] = 50;
        SettingsArray[2] = 40;
        SettingsArray[3] = 35;
        std::cout << "Unable To Read default Settings /etc/fan_ctrl/fan_settings" << std::endl;
        return;
    }
    fscanf(settingsFile, "%i\n%i\n%i\n%i", 
        &SettingsArray[0], // Interval Wait
        &SettingsArray[1], // Max Temp
        &SettingsArray[2], // Mid Temp
        &SettingsArray[3]); // Min Temp
    // printf("The temperature is %6.3f C.\n", T);
    fclose(settingsFile);
}


int main(void)
{
    double CPU_Temp;
	wiringPiSetupSys();

	pinMode(POWERLED, OUTPUT);
    pinMode(FAN_CTRL, OUTPUT);
    pinMode(ENABLE_PWM, OUTPUT);

    // To read from File
    int Settings[4];

    // Read from Init File
    GetInitConfigs(Settings);

    // To simplify the logic later...
    int MinsToWait, TempMax, TempMiddle, TempLow;
    MinsToWait = Settings[0];
    TempMax = Settings[1];
    TempMiddle = Settings[2];
    TempLow = Settings[3];

    std::cout << "Started Fan Control" << std::endl;

    digitalWrite(FAN_CTRL, TRUE);
    std::cout << "Enable Fan" << std::endl;
    digitalWrite(ENABLE_PWM, TRUE);
    std::cout << "Enable Fan PWM" << std::endl;


    // Temp Control Loop

   while(true) {

        //ToggleStatus(POWERLED); Debug Only
        CPU_Temp = GetTempValue();

        if (CPU_Temp != -150) {
            std::cout << "Current Temp Value: " << CPU_Temp <<" *c" <<std::endl;
            if (CPU_Temp > TempMax) {
                std::cout << "Set the Fans At MAX Power" << std::endl;
                digitalWrite(ENABLE_PWM, FALSE);
                digitalWrite(FAN_CTRL, TRUE);

            }
            else if (CPU_Temp <= TempMax && CPU_Temp > TempMiddle) {
                std::cout << "Set the Fans at Middle Power" << std::endl;
                digitalWrite(ENABLE_PWM, TRUE);
                digitalWrite(FAN_CTRL, TRUE);

            }
            else {
                std::cout << "Set Fans OFF" << std::endl;
                digitalWrite(FAN_CTRL, FALSE);
                digitalWrite(ENABLE_PWM, FALSE);
            }
            waitMins(MinsToWait);
        }

    }
    
    std::cout << "Finished Loop";



}



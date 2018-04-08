#ifndef ADD_testH
#define ADD_testH

#include "Arduino.h"
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>
#include <i2c_t3.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_TSL2561_U.h"

//INTERFACE SETTINGS 
#define WAIT_TIME 100 //Define timeout for user input in seconds
#define MAX_LENGTH 50 //maximum length of filename used for logging
#define VCNL4010 0x13 //i2c address for VCNL4010 ambient light sensor

//LCD pins
extern int mosi_pin;
extern int rst_pin;
extern int a0_pin;
extern int cs_pin;
extern int scl_pin;
//Control pins
extern int b1_pin;
extern int b2_pin;
extern int b3_pin;
extern int pot_pin;
//Measure pins
extern int ain_pin;
//Constants
extern const double A; 
extern const double B;
//Logging parameters. TODO: Store in EEPROM
extern int mem_logInterval;
extern int mem_logTime;
extern char mem_filename[MAX_LENGTH];
extern File mem_logfile;
//tsl2561 parameters
#define TSL2561_ADDR 0x29
extern Adafruit_TSL2561_Unified tsl;
//Calibration parameters (initialized as A = 1, B = 0)
//Calibration format: lux = A*l + B
#define TSL2561_CAL_A 1
#define TSL2561_CAL_B 0


//HARDWARE FUNCTIONS

void logger_init(); //Initialize correct settings for hardware. Run this in setup()

double measure_vin(); //Estimate real voltage value of between input - and input +

double measure_vin_n(int n); //Same as measure_vin(), but with mean of n samples to minimize noise

double measure_vout(); //Measure voltage coming directly at ADC pin

double measure_vout_n(int n); //Same as measure_vout(), but with mean of n samples to minimize noise

void tsl2561_init(); //Initializes light sensor tsl2561

void tsl2561_configure(); //Configures the light sensor tsl2561

void tsl2561_displaySensorDetails(); // Displays some basic information on this sensor

uint32_t measure_lux(); //Measures lux value from tsl2561 sensor through i2c

//INTERFACE FUNCTIONS

void serial_interface(); //Every function of the power monitor should be possible to perform through this Serial interface

unsigned long ask_logging_params(); //Ask logging settings from user, store in EEPROM and return total number of measurements

int askInt(char* str); //In progress. Don't use....

void ask_time(); //What time is it? Ask the user!

int yes_or_not(); //Interface for verifying from user whether he wants to continue or not

//LOGGING FUNCTIONS

void init_logging(); //After ask_logging_params performs all required actions for one logging session.

void logging(int interval, unsigned long n); //Measures voltage and records the result in the logfile at interval n times

void writeData(int i); //Writes a comma-separated string with i:th index, date, time and measurement value to logfile


#endif
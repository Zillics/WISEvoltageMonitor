#ifndef ADD_testH
#define ADD_testH

#include "Arduino.h"

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
extern const double k; //voltage divider constant (hand calibrated)

void logger_init(); //Initialize correct settings for hardware. Run this in setup()

double measure_vin(); //Estimate real voltage value of between input - and input +

void serial_interface(); //Every function of the power monitor should be possible to perform through this Serial interface

void test_function();

void init_logging();

#endif
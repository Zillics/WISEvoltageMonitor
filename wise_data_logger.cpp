#include "wise_data_logger.h"
//LCD pins
int mosi_pin = 0;
int rst_pin = 1;
int a0_pin = 2;
int cs_pin = 31;
int scl_pin = 32;
//Control pins
int b1_pin = 24;
int b2_pin = 25;
int b3_pin = 26;
int pot_pin = 15;
//Measure pins
int ain_pin = 14;
//Constants
const double k = 1.64978903; //voltage divider constant (hand calibrated)

//Initialize correct settings for hardware 

void logger_init(){
  //Set ADC resolution to 13 bit
  analogReadResolution(13);
  //Pin initialization
  pinMode(mosi_pin,OUTPUT);
  pinMode(rst_pin,OUTPUT);
  pinMode(a0_pin,OUTPUT);
  pinMode(cs_pin,OUTPUT);
  pinMode(scl_pin,OUTPUT);
  pinMode(b1_pin,INPUT);
  pinMode(b2_pin,INPUT);
  pinMode(b3_pin,INPUT);
  pinMode(pot_pin,INPUT);
  pinMode(ain_pin,INPUT);
}

//Estimate real voltage value of between input - and input +
//Vin: input to op amp
//Vout: output after op amp and voltage divider
double measure_vin(){
	int ain = analogRead(ain_pin);
	double Vout = ((double)ain/8191.0)*3.3; //Convert from 13 bit -> real voltage value
	double Vin = Vout*k; //After voltage divider
	return Vin;
}


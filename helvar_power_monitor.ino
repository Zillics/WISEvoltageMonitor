#include "wise_data_logger.h"

void setup() {
  logger_init();
  //Serial initialization for troubleshooting
  Serial.begin(9600);
}

//Valuesl

int pot = -1;
int ain = -1;
double Vin = -1;
double Vout = -1;
//Constants

void loop() {
serial_interface();
}


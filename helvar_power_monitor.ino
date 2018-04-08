#include "wise_data_logger.h"

void setup() {
  delay(200);
  //Serial initialization for troubleshooting
  Serial.begin(9600);
  logger_init();
}

//Valuesl

int pot = -1;
int ain = -1;
double Vin = -1;
double Vout = -1;
uint32_t l = 0;
//Constants

void loop() {
serial_interface();
//Serial.println(measure_lux());
//delay(200);
}


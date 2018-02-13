#include "wise_data_logger.h"

void setup() {
  logger_init();
  //Serial initialization for troubleshooting
  Serial.begin(9600);
}

//Values
int pot = -1;
int ain = -1;
double Vin = -1;
double Vout = -1;
//Constants


void loop() {
  while(true){
    Serial.print("Vin: ");
    Serial.println(measure_vin(),3);
    delay(500);
  }
}
  


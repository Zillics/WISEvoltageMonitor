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
const double k = 1.64978903; //voltage divider constant (hand calibrated) TODO: Estimate properly (fit a line)

//Values in memory
char mem_filename[] = "";
int mem_log_interval;
int mem_log_time;
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

void serial_interface(){
	int inByte = 0; //Stores the Serial command from user input
	while(true){
		Serial.println("/////////////////////////////////////////////////////////");
		//Welcome message and instructions
		Serial.println("This is the Serial interface of WISE power monitor. Press ENTER to reset and return to this window.");
		Serial.println("Available commands:");
		Serial.println("l : start logging; t : test");
		Serial.print("Enter command and press enter: ");
		Serial.read(); //Empty Serial buffer
		while(Serial.available() == 0){} //Wait until user input
		inByte = Serial.read(); //Stores user input (one letter)
		Serial.read(); //Clean Serial buffer from ENTER
		Serial.println(char(inByte));
		switch(inByte) {
			//case 'l': start_logging();
			case 't': test_function();
			break;
			case 'l': init_logging();
			default : Serial.println("NO SUCH COMMAND");
			break;
		}	
	}
}

void init_logging(){
	//Interface settings
	int wait_time = 100; //Define timeout for user input in seconds
	Serial.setTimeout(100*wait_time); //Set timeout for user input 
	int max_length = 50; //maximum length of filename in letters
	//User input containers
	char filename[] = "";
	int log_interval; //Logging interval in seconds
	const int max_interval = 1000; //Maximum acceptable value for logging interval
	char days[] = "";
	char hours[] = "";
	bool proper_interval = false; //Determines whether user has chosen proper logging interval
	//Start interface
	Serial.println("***********Logging***************");
	Serial.println();
	Serial.print("Enter filename: ");
	size_t name_len = Serial.readBytesUntil(10,filename, max_length); //terminates with newline (ASCII: 10)
	Serial.println(filename);
	while(!proper_interval){
		Serial.print("Enter logging interval in seconds: ");
		while(Serial.available() == 0){} //Wait until user input
		log_interval = Serial.parseInt(); //Store user input
		proper_interval = log_interval < max_interval && log_interval > 0; //Check whether this is an acceptable interval
		Serial.println(log_interval);
		if(!proper_interval){ Serial.println("INCORRECT VALUE, CHOOSE BETTER!"); }
	}
	Serial.println();
	Serial.print(filename);
	Serial.print("Logging successful! Written to file: ");
	Serial.println(filename);
}

void test_function(){
	Serial.println("THIS IS A TEST. WOW!");
	Serial.read();
}


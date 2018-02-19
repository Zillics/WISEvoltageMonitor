#include "wise_data_logger.h"
//SD card constants
const int chipSelect = BUILTIN_SDCARD;
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
const double k = 1.6468934199; //voltage divider constant (hand calibrated) TODO: Estimate properly (fit a line)

//Values in memory TODO: implement this in a smart way (EEPROM??)
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

double measure_vout(){
	int ain = analogRead(ain_pin);
	double Vout = ((double)ain/8191.0)*3.3; //Convert from 13 bit -> real voltage value
	return Vout;
}

void serial_interface(){
	int inByte = 0; //Stores the Serial command from user input
	while(true){
		Serial.println("/////////////////////////////////////////////////////////");
		//Welcome message and instructions
		Serial.println("This is the Serial interface of WISE power monitor. Press ENTER to reset and return to this window.");
		Serial.println("Available commands:");
		Serial.println("l : start logging, t : test");
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
	//User input containers
	char filename[] = "";
	unsigned long n = 0; //Total number of measurements. To be calculated after user input
	//Start interface
	Serial.println("***********Logging***************");
	n = ask_logging_params();
	if(n > 0){
		Serial.println("STARTING LOGGING.....");
		start_logging(mem_filename, mem_log_interval, n);
		Serial.println("LOGGING ENDED");
		return;
	}
	else{
		return;
	}
}

void start_logging(char* filename, int interval, unsigned long n){
	if(!SD_verify()){
		return;
	}
	File logFile = SD.open(filename, FILE_WRITE);
	if (!logFile) {
		Serial.print("Error opening file");
		Serial.println(filename);
		return;
	}
	else{
		Serial.println("Opening file successful. Logging started...");
		logging(logFile, interval, n);
		logFile.close();
	}
}

void logging(File logfile, int interval,unsigned long n){
	//Container for data for each measurement
	char* measData;
	//Performing measurements n times
	for(int i = 0;i < n; i++){
		measData = fetchData(i);
		Serial.println(measData);
		//logfile.println(measData);
		delay(1000*interval);
	}
}

//Creates a string for the measuring status in the following form;
//"[Number of measurement],[Date and time of measurement], [Measurement value in V]"
char* fetchData(int i){
	int ENOUGH = (int)((ceil(log10(i))+1)*sizeof(char));
	char str[ENOUGH];
	sprintf(str,"%d",i);
	//strcat(str,)
	return str;
}

void test_function(){
	Serial.println("THIS IS A TEST. WOW!");
	Serial.read();
}
	/*
  SD card datalogger
 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe
 modified 19 Feb 2018
 by Erik Zilliacus
 */
bool SD_verify(){
	Serial.print("Initializing SD card...");
	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect)) {
		Serial.println("Card failed, or not present");
		// don't do anything more:
		return false;
	}
	Serial.println("card initialized.");
	return true;
}

bool yes_or_not(){
	int usr_input = 0;
	while(true){
		Serial.println("Do you want to continue? (y/n)");
		while(Serial.available() == 0){} //Wait until user input
		usr_input = Serial.read(); //Stores user input (one letter)
		Serial.read(); //Clean Serial buffer from ENTER
		Serial.println(char(usr_input));
		switch(usr_input){
			case 'y' : return true;
			case 'Y' : return true;
			case 'n' : return false;
			case 'N' : return false;
			default : Serial.println("Try again!");
		}
	}
}
//Asks for logging parameters: Timespan (days+hours+minutes) and measurement interval in seconds
//Returns calculated total number of measurements based on user input.
//Returns 0 if params invalid or user doesnt want to continue
unsigned long ask_logging_params(){
	//INTERFACE SETTINGS 
	int wait_time = 100; //Define timeout for user input in seconds
	int max_length = 50; //maximum length of filename in letters
	Serial.setTimeout(1000*wait_time); //Set timeout for user input
	//LOGGING PARAMETERS
	int log_interval; //Logging interval in seconds
	//Logging timespan in days + hours + minutes
	int days = 0;
	int hours = 0;
	int minutes = 0;
	int measurements = 0;
	//INTERFACE
	Serial.println();
	Serial.print("Enter filename: ");
	size_t name_len = Serial.readBytesUntil(10,mem_filename, max_length); //terminates with newline (ASCII: 10)
	Serial.println(mem_filename);
	Serial.print("Enter logging interval in seconds: ");
	while(Serial.available() == 0){} //Wait until user input
	mem_log_interval = Serial.parseInt(); //Store user input
	if(mem_log_interval <= 0){
		Serial.println("INVALID INTERVAL. TRY AGAIN");
		return 0;
	}
	Serial.println(mem_log_interval);
	Serial.println("LOGGING TIMESPAN: ");
	Serial.print("How many days? ");
	days = Serial.parseInt();
	Serial.println(days);
	Serial.print("How many hours?");
	hours = Serial.parseInt();
	Serial.println(hours);
	Serial.print("How many minutes?");
	minutes = Serial.parseInt();
	Serial.println(minutes);
	Serial.print("Logging setup for ");
	Serial.print(days);
	Serial.print(" days, ");
	Serial.print(hours);
	Serial.print(" hours. and ");
	Serial.print(minutes);
	Serial.print(" minutes. Logging interval: ");
	Serial.print(mem_log_interval);
	Serial.println(" seconds.");
	if(!yes_or_not()){
		return 0;
	}
	else{
		unsigned long n = (60*60*24*days)/mem_log_interval + (60*60*hours)/mem_log_interval + (minutes*60)/mem_log_interval;
		//CALCULATING TOTAL NUMBER OF MEASUREMENTS BASED ON USER INPUT
		return n;
	}
}

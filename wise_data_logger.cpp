#include "wise_data_logger.h"

/*
//LCD pins
int mosi_pin = 0;
int rst_pin = 1;
int a0_pin = 2;
int cs_pin = 31;
int scl_pin = 32;
*/
//Measure pins
int ain_pin = 14;
//Constants
//Calibrated values (using LS estimation) for estimating vin = vout*A + B 
//NOTE:most optimal for values 3.3 V - 5 V 
const double A = 1.65953740047; 
const double B = -0.0291635521063; 
//Values in memory TODO: implement this in a smart way (EEPROM??)
char mem_filename[MAX_LENGTH];
int mem_logInterval;
int mem_logTime;
File mem_logfile;

//***********HARDWARE FUNCTIONS*************

//Initialize correct settings for hardware 
void logger_init(){
  //Set ADC resolution to 13 bit
  analogReadResolution(13);
  //Pin initialization
  pinMode(ain_pin,INPUT);
}

//Estimate real voltage value of between input - and input +
//Vin: input to op amp
//Vout: output after op amp and voltage divider
double measure_vin(){
	int ain = analogRead(ain_pin);
	double Vout = ((double)ain/8191.0)*3.3; //Convert from 13 bit -> real voltage value
	double Vin = Vout*A + B; //After voltage divider
	return Vin;
}
//Same as measure_vin(), but with mean of n samples to minimize noise
double measure_vin_n(int n){
	double Vout = measure_vout_n(n);
	double Vin = Vout*A + B;
	return Vin;
}

double measure_vout(){
	int ain = analogRead(ain_pin);
	double Vout = ((double)ain/8191.0)*3.3; //Convert from 13 bit -> real voltage value
	return Vout;
}
//Same as measure_vout(), but with mean of n samples to minimize noise
double measure_vout_n(int n){
	double sum = 0;
	for(int i = 0;i < n;i++){
		sum += measure_vout();
	}
	double mean = sum/(double)n;
	return mean;
}

//***********INTERFACE FUNCTIONS*************


void serial_interface(){
	//Initialize settings
	Serial.setTimeout(1000*100); //Set timeout for user input
	int inByte = 0; //Stores the Serial command from user input
	while(true){
		//Welcome message and instructions
		Serial.println(F("This is the Serial interface of WISE power monitor"));
		Serial.println(F("Available commands:"));
		Serial.println(F("l : start logging, m : measure vin"));
		Serial.print(F("Enter command and press enter: "));
		Serial.read(); //Empty Serial buffer
		while(Serial.available() == 0){} //Wait until user input
		inByte = Serial.read(); //Stores user input (one letter)
		Serial.read(); //Clean Serial buffer from ENTER
		Serial.println(char(inByte));
		switch(inByte) {
			//case 'l': start_logging();
			case 'l': init_logging();
				break;
			case 'm': Serial.println(measure_vin(),3);
				break;
			default : Serial.println(F("NO SUCH COMMAND"));
				break;
		}	
	}
}

//Asks for logging parameters: Timespan (days+hours+minutes) and measurement interval in seconds
//Returns calculated total number of measurements based on user input.
//Returns 0 if params invalid or user doesnt want to continue
unsigned long ask_logging_params(){
	//LOGGING PARAMETERS
	//Logging timespan in days + hours + minutes
	int days = 0;
	int hours = 0;
	int minutes = 0;
	int measurements = 0;
	//INTERFACE
	ask_time();
	delay(100);
	Serial.println();
	Serial.print(F("Enter filename: "));
	Serial.read();
	size_t name_len = Serial.readBytesUntil(10,mem_filename,50); //terminates with newline (ASCII: 10)
	Serial.println(mem_filename);
	Serial.print(F("Enter logging interval in seconds: "));
	while(Serial.available() == 0){} //Wait until user input
	mem_logInterval = Serial.parseInt(); //Store user input
	delay(100);
	if(mem_logInterval <= 0){
		Serial.println(F("INVALID INTERVAL. TRY AGAIN"));
		return 0;
	}
	delay(100);
	Serial.println(mem_logInterval);
	Serial.println(F("LOGGING TIMESPAN: "));
	Serial.print(F("How many days? "));
	days = Serial.parseInt();
	Serial.println(days);
	delay(100);
	Serial.print(F("How many hours?"));
	hours = Serial.parseInt();
	Serial.println(hours);
	delay(100);
	Serial.print(F("How many minutes?"));
	minutes = Serial.parseInt();
	delay(100);
	Serial.println(minutes);
	Serial.print(F("Logging setup for "));
	Serial.print(days);
	Serial.print(F(" days, "));
	Serial.print(hours);
	Serial.print(F(" hours. and "));
	Serial.print(minutes);
	Serial.print(F(" minutes. Logging interval: "));
	Serial.print(mem_logInterval);
	Serial.println(F(" seconds."));
	delay(100);
		//CALCULATING TOTAL NUMBER OF MEASUREMENTS BASED ON USER INPUT
	unsigned long n = (60*60*24*days)/mem_logInterval + (60*60*hours)/mem_logInterval + (minutes*60)/mem_logInterval;
	Serial.println(n);

	return n;
}
//What time is it? Ask the user!
void ask_time(){
	Serial.println(F("SETTING TIME"));
	Serial.print(F("MONTH: "));
	int month = Serial.parseInt();
	Serial.println(month);
	Serial.print(F("DAY: "));
	int day = Serial.parseInt();
	Serial.println(day);
	Serial.print(F("HOUR: "));
	int hour = Serial.parseInt();
	Serial.println(hour);
	Serial.print(F("MINUTE: "));
	int minute = Serial.parseInt();
	Serial.println(minute);
	int second = 0;
	int year = 2018;
	setTime(hour, minute,second, day, month, year);
}
//SUDDENLY STOPPED WORKING.... Don't use
int askInt(char* str){
	int input;
	Serial.print(str);
	input = Serial.parseInt();
	Serial.println(input);
	Serial.flush();
	return input; 
}
//SUDDENLY STOPPED WORKING....Don't use
int yes_or_not(){
	int usr_input = 0;
	while(true){
		Serial.println(F("Do you want to continue? (y/n)"));
		while(Serial.available() == 0){} //Wait until user input
		usr_input = Serial.read(); //Stores user input (one letter)
		Serial.read(); //Clean Serial buffer from ENTER
		Serial.println(char(usr_input));
		switch(usr_input){
			case 'y' : return 1;
			case 'Y' : return 1;
			case 'n' : return 0;
			case 'N' : return 0;
			default : Serial.println(F("Try again!"));
		}
	}
}

//***********LOGGING FUNCTIONS*************

void init_logging(){
	const int chipSelect = BUILTIN_SDCARD; //Select the built-in SD card for Teensy 3.5
	Serial.print(F("Initializing SD card..."));
	// see if the card is present and can be initialized:
	if (!SD.begin(chipSelect)) {
		Serial.println(F("Card failed, or not present"));
	}
	else{
		Serial.println(F("card initialized."));
		//User input containers
		unsigned long n = 0; //Total number of measurements. To be calculated after user input
		//Start interface
		Serial.println(F("***********Logging***************"));
		n = ask_logging_params(); //Get total number of measurements from user input
		delay(100);
		if(n > 0){
			Serial.println(F("STARTING LOGGING....."));
			delay(100);
			logging(mem_logInterval, n);
			Serial.println(F("LOGGING ENDED"));
		}
		else{
			Serial.println(F("Invalid logging parameters. Try again!"));
		}
	}
}

void logging(int interval,unsigned long n){
	//Performing measurements n times
	for(int i = 0;i < n; i++){
		writeData(i);
		//logfile.println(measData);
		delay(1000*interval);
	}
}

//Creates a string for the measuring status in the following form;
//"[Number of measurement],[Date and time of measurement], [Measurement value in V]"
void writeData(int i){
	delay(100);
	File logFile = SD.open(mem_filename, FILE_WRITE);//Open file. FILE_WRITE starts writing at end of file
	delay(100);
	if (!logFile) {
		Serial.print(F("Error opening file"));
		Serial.println(mem_filename);
	}
	else{
		//Initializing string starting with index i
		//int ENOUGH = ((ceil(log10(i))+1)*sizeof(char));
		int ENOUGH = 10;//Assuming index never exceeds 10 
		char index[ENOUGH];
		delay(10);
		sprintf(index,"%d",i);
		delay(50);
		//Taking time and adding that to string
		char h[3];
		sprintf(h,"%d",hour());
		char min[3];
		sprintf(min,"%d",minute());
		char s[3];
		sprintf(s,"%d",second());
		char d[3];
		sprintf(d,"%d",day());
		char mon[3];
		sprintf(mon,"%d",month());
		char y[5];
		sprintf(y,"%d",year());
		//Performing measurement and adding result to string
		char v[6];
		sprintf(v,"%0.3f",measure_vin_n(100));
		//TODO:Writing string to logfile
		char log_i[40];
		strcpy(log_i,index);
		strcat(log_i,",");
		strcat(log_i,h);
		strcat(log_i,":");
		strcat(log_i,min);
		strcat(log_i,":");
		strcat(log_i,s);
		strcat(log_i,",");
		strcat(log_i,d);
		strcat(log_i,"/");
		strcat(log_i,mon);
		strcat(log_i,"/");
		strcat(log_i,y);
		strcat(log_i,",");
		strcat(log_i,v);
		delay(10);
		Serial.println(log_i);
		delay(100);
		logFile.println(log_i); //Write string + newline to logfile
		delay(10);
		logFile.close(); //Close file when ready
		delay(10);
	}
}
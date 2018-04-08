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
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR, 12345);
//***********HARDWARE FUNCTIONS*************

//Initialize correct settings for hardware 
void logger_init(){
  //Set ADC resolution to 13 bit
  analogReadResolution(13);
  //Pin initialization
  pinMode(ain_pin,INPUT);
  //Light sensor initialization
  tsl2561_init();
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
//Initializes light sensor tsl2561
void tsl2561_init(){
	delay(200);
	/* Initialise the sensor */
	//use tsl.begin() to default to Wire, 
	//tsl.begin(&Wire2) directs api to use Wire2, etc.
	if(!tsl.begin())
	{
		/* There was a problem detecting the TSL2561 ... check your connections */
		Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
		while(1);
  }
  
  /* Display some basic information on this sensor */
tsl2561_displaySensorDetails();
  
  /* Setup the sensor gain and integration time */
tsl2561_configure();
  
  /* We're ready to go! */
Serial.println("");
} 
//Configures the light sensor tsl2561
void tsl2561_configure(){
	/* You can also manually set the gain or enable auto-gain support */
	// tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
	// tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
	tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

	/* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
	//tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
	// tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
	tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

	/* Update these values depending on what you've set above! */  
	Serial.println("------------------------------------");
	Serial.print  ("Gain:         "); Serial.println("Auto");
	Serial.print  ("Timing:       "); Serial.println("13 ms");
	Serial.println("------------------------------------");
}
void tsl2561_displaySensorDetails(void)
{
	sensor_t sensor;
	tsl.getSensor(&sensor);
	Serial.println("------------------------------------");
	Serial.print  ("Sensor:       "); Serial.println(sensor.name);
	Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
	Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
	Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
	Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
	Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
	Serial.println("------------------------------------");
	Serial.println("");
	delay(500);
}
//Measure lux value from tsl2561 sensor through i2c
uint32_t measure_lux(){
	/* Get a new sensor event */ 
	sensors_event_t event;
	tsl.getEvent(&event);

	/* Display the results (light is measured in lux) */
	if (event.light)
	{
		//Serial.print(event.light); Serial.println(" lux");
		return(TSL2561_CAL_A*event.light + TSL2561_CAL_B);
	}
	else
	{
	/* If event.light = 0 lux the sensor is probably saturated
	   and no reliable data could be generated! */
		Serial.println("Sensor overload");
		return(0);
	}
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
		Serial.print(F("Error opening file "));
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
		char l[6];
		sprintf(l,"%ld",measure_lux());
		char log_i[46];
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
		strcat(log_i,",");
		strcat(log_i, l);
		delay(10);
		Serial.println(log_i);
		delay(100);
		logFile.println(log_i); //Write string + newline to logfile
		delay(10);
		logFile.close(); //Close file when ready
		delay(10);
	}
}
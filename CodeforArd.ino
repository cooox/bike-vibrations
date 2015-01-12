//libaries 
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) 
HardwareSerial mySerial = Serial1;
#else
SoftwareSerial mySerial(8, 7);
#endif

Adafruit_GPS GPS(&mySerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  false

int stamp =0;

double latitude;
double longitude; 
int day;
int month; 
int year;
int hour; 
int minute;
int seconds;



double acc_X;
double acc_Y;
double acc_Z;

boolean first = true;

File myFile;


// change this to match your SD shield or module;
//     Adafruit SD shields and modules: pin 10
const int chipSelect = 10;

void displaySensorDetails(void)
{
  sensor_t sensor;
  accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); 
  Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); 
  Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); 
  Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); 
  Serial.print(sensor.max_value); 
  Serial.println(" m/s^2");
  Serial.print  ("Min Value:    "); 
  Serial.print(sensor.min_value); 
  Serial.println(" m/s^2");
  Serial.print  ("Resolution:   "); 
  Serial.print(sensor.resolution); 
  Serial.println(" m/s^2");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}



const int vibsens_high1 = 6; //PIN empfindlicher Sensor, 18010, kurz
const int vibsens_high2 = 2;
const int vibsens_high3 = 0;
const int vibsens_low1 = 5;  //PIN weniger empfindlicher Sensor, 18030, lang
const int vibsens_low2 = 4;
const int vibsens_low3 = 3;


void setup() {

  Serial.begin(9600);
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(SS, OUTPUT);

  //  if (!SD.begin(chipSelect)) {
  if (!SD.begin(chipSelect, 11, 12, 13)) {
    Serial.println("-initialization failed!");
    return;
  }
  Serial.println("+initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("data.txt", FILE_WRITE);


  //GPS block--------------------------------------------------------------------
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.println("Adafruit GPS library basic test!");

  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time

  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);


  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);


  pinMode(vibsens_high1, INPUT_PULLUP);
  pinMode(vibsens_high2, INPUT_PULLUP);
  pinMode(vibsens_high3, INPUT_PULLUP);
  pinMode(vibsens_low1, INPUT_PULLUP);
  pinMode(vibsens_low2, INPUT_PULLUP);
  pinMode(vibsens_low3, INPUT_PULLUP);

  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }

  /* Display some basic information on this sensor */
  displaySensorDetails();


}

uint32_t timer = millis();

void loop() {

  //GPS block-------------------------------------------------------------------
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!

  if ((c) && (GPSECHO))
  Serial.write(c); 

  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another

  }

  if (first){
    first = false;
    delay(5000);
  }
  
  /* Get a new sensor event */
  sensors_event_t event; 
  accel.getEvent(&event);
  
  acc_X = event.acceleration.x;
 acc_Y = event.acceleration.y;
 acc_Z =event.acceleration.x;

  // approximately every minute, print out the current stats
  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 second, print out the current stats
  if (millis() - timer > 2000) { 
    Serial.println("INSIDE");
    timer = millis(); // reset the timer
    latitude = GPS.latitude;
    longitude = GPS.longitude;
    day = GPS.day;
    month= GPS.month;
    year = GPS.year;
    hour = GPS.hour;
    minute = GPS.minute;
    seconds = GPS.seconds;  

  }
  


  String fileInput = "";


  if (digitalRead(vibsens_high1) == LOW){
    Serial.println("schwache Vibration1");

    // Display the results (acceleration is measured in m/s^2) 
    Serial.print("X: "); 
    Serial.print(acc_X); 
    Serial.print("  ");
    Serial.print("Y: "); 
    Serial.print(acc_Y); 
    Serial.print("  ");
    Serial.print("Z: "); 
    Serial.print(acc_Z); 
    Serial.print("  ");
    Serial.println("m/s^2 ");


    stamp++;
    fileInput = "";
    fileInput.concat(stamp);
    fileInput.concat(",");
    myFile.print(fileInput);
    fileInput = "";


    fileInput.concat(acc_X);
    fileInput.concat(",");
    fileInput.concat(acc_Y);
    fileInput.concat(",");
    fileInput.concat(acc_Z);
    fileInput.concat(",");
    fileInput.concat(latitude);
    fileInput.concat(",");
    fileInput.concat(longitude);
    fileInput.concat(",");
    fileInput.concat(day);
    fileInput.concat(",");
    fileInput.concat(month);
    fileInput.concat(",");
    fileInput.concat(year);
    fileInput.concat(",");
    fileInput.concat(hour);
    fileInput.concat(",");
    fileInput.concat(minute);
    fileInput.concat(",");
    fileInput.concat(seconds);
    fileInput.concat(",");
    fileInput.concat(millis());
    fileInput.concat(",");
    fileInput.concat("1.1");

    myFile.println(fileInput);
    Serial.print(fileInput);
    //myFile.flush();



  }
  
 if (digitalRead(vibsens_high2) == LOW){
    Serial.println("schwache Vibration2");

    // Display the results (acceleration is measured in m/s^2) 
    Serial.print("X: "); 
    Serial.print(acc_X); 
    Serial.print("  ");
    Serial.print("Y: "); 
    Serial.print(acc_Y); 
    Serial.print("  ");
    Serial.print("Z: "); 
    Serial.print(acc_Z); 
    Serial.print("  ");
    Serial.println("m/s^2 ");


    stamp++;
    fileInput = "";
    fileInput.concat(stamp);
    fileInput.concat(",");
    myFile.print(fileInput);
    fileInput = "";


    fileInput.concat(acc_X);
    fileInput.concat(",");
    fileInput.concat(acc_Y);
    fileInput.concat(",");
    fileInput.concat(acc_Z);
    fileInput.concat(",");
    fileInput.concat(latitude);
    fileInput.concat(",");
    fileInput.concat(longitude);
    fileInput.concat(",");
    fileInput.concat(day);
    fileInput.concat(",");
    fileInput.concat(month);
    fileInput.concat(",");
    fileInput.concat(year);
    fileInput.concat(",");
    fileInput.concat(hour);
    fileInput.concat(",");
    fileInput.concat(minute);
    fileInput.concat(",");
    fileInput.concat(seconds);
    fileInput.concat(",");
    fileInput.concat(millis());
    fileInput.concat(",");
    fileInput.concat("1.2");

    myFile.println(fileInput);
    Serial.print(fileInput);
   // myFile.flush();



  }
  
 if (digitalRead(vibsens_high3) == LOW){
    Serial.println("schwache Vibration3");

    //Display the results (acceleration is measured in m/s^2) 
    Serial.print("X: "); 
    Serial.print(acc_X); 
    Serial.print("  ");
    Serial.print("Y: "); 
    Serial.print(acc_Y); 
    Serial.print("  ");
    Serial.print("Z: "); 
    Serial.print(acc_Z); 
    Serial.print("  ");
    Serial.println("m/s^2 ");


    stamp++;
    fileInput = "";
    fileInput.concat(stamp);
    fileInput.concat(",");
    myFile.print(fileInput);
    fileInput = "";


    fileInput.concat(acc_X);
    fileInput.concat(",");
    fileInput.concat(acc_Y);
    fileInput.concat(",");
    fileInput.concat(acc_Z);
    fileInput.concat(",");
    fileInput.concat(latitude);
    fileInput.concat(",");
    fileInput.concat(longitude);
    fileInput.concat(",");
    fileInput.concat(day);
    fileInput.concat(",");
    fileInput.concat(month);
    fileInput.concat(",");
    fileInput.concat(year);
    fileInput.concat(",");
    fileInput.concat(hour);
    fileInput.concat(",");
    fileInput.concat(minute);
    fileInput.concat(",");
    fileInput.concat(seconds);
    fileInput.concat(",");
    fileInput.concat(millis());
    fileInput.concat(",");
    fileInput.concat("1.3");

    myFile.println(fileInput);
    Serial.print(fileInput);
   // myFile.flush();



  }
  
  if (digitalRead(vibsens_low1) == LOW){
    Serial.println("starke Vibration1");
    // Display the results (acceleration is measured in m/s^2) 
    Serial.print("X: "); 
    Serial.print(acc_X); 
    Serial.print("  ");
    Serial.print("Y: "); 
    Serial.print(acc_Y); 
    Serial.print("  ");
    Serial.print("Z: "); 
    Serial.print(acc_Z); 
    Serial.print("  ");
    Serial.println("m/s^2 ");

    stamp++;
    fileInput = "";
    fileInput.concat(stamp);
    fileInput.concat(",");
    myFile.print(fileInput);
    fileInput = "";

    fileInput.concat(acc_X);
    fileInput.concat(",");
    fileInput.concat(acc_Y);
    fileInput.concat(",");
    fileInput.concat(acc_Z);
    fileInput.concat(",");
    fileInput.concat(latitude);
    fileInput.concat(",");
    fileInput.concat(longitude);
    fileInput.concat(",");
    fileInput.concat(day);
    fileInput.concat(",");
    fileInput.concat(month);
    fileInput.concat(",");
    fileInput.concat(year);
    fileInput.concat(",");
    fileInput.concat(hour);
    fileInput.concat(",");
    fileInput.concat(minute);
    fileInput.concat(",");
    fileInput.concat(seconds);
    fileInput.concat(",");
    fileInput.concat(millis());
    fileInput.concat(",");
    fileInput.concat("2.1");

    myFile.println(fileInput);
    Serial.print(fileInput);
    //myFile.flush();
  }
  
  if (digitalRead(vibsens_low2) == LOW){
    Serial.println("starke Vibration2");
    // Display the results (acceleration is measured in m/s^2) 
    Serial.print("X: "); 
    Serial.print(acc_X); 
    Serial.print("  ");
    Serial.print("Y: "); 
    Serial.print(acc_Y); 
    Serial.print("  ");
    Serial.print("Z: "); 
    Serial.print(acc_Z); 
    Serial.print("  ");
    Serial.println("m/s^2 ");

    stamp++;
    fileInput = "";
    fileInput.concat(stamp);
    fileInput.concat(",");
    myFile.print(fileInput);
    fileInput = "";

    fileInput.concat(acc_X);
    fileInput.concat(",");
    fileInput.concat(acc_Y);
    fileInput.concat(",");
    fileInput.concat(acc_Z);
    fileInput.concat(",");
    fileInput.concat(latitude);
    fileInput.concat(",");
    fileInput.concat(longitude);
    fileInput.concat(",");
    fileInput.concat(day);
    fileInput.concat(",");
    fileInput.concat(month);
    fileInput.concat(",");
    fileInput.concat(year);
    fileInput.concat(",");
    fileInput.concat(hour);
    fileInput.concat(",");
    fileInput.concat(minute);
    fileInput.concat(",");
    fileInput.concat(seconds);
    fileInput.concat(",");
    fileInput.concat(millis());
    fileInput.concat(",");
    fileInput.concat("2.2");

    myFile.println(fileInput);
    Serial.print(fileInput);
    //myFile.flush();
  }
  
  if (digitalRead(vibsens_low3) == LOW){
    Serial.println("starke Vibration3");
    // Display the results (acceleration is measured in m/s^2) 
    Serial.print("X: "); 
    Serial.print(acc_X); 
    Serial.print("  ");
    Serial.print("Y: "); 
    Serial.print(acc_Y); 
    Serial.print("  ");
    Serial.print("Z: "); 
    Serial.print(acc_Z); 
    Serial.print("  ");
    Serial.println("m/s^2 ");

    stamp++;
    fileInput = "";
    fileInput.concat(stamp);
    fileInput.concat(",");
    myFile.print(fileInput);
    fileInput = "";

    fileInput.concat(acc_X);
    fileInput.concat(",");
    fileInput.concat(acc_Y);
    fileInput.concat(",");
    fileInput.concat(acc_Z);
    fileInput.concat(",");
    fileInput.concat(latitude);
    fileInput.concat(",");
    fileInput.concat(longitude);
    fileInput.concat(",");
    fileInput.concat(day);
    fileInput.concat(",");
    fileInput.concat(month);
    fileInput.concat(",");
    fileInput.concat(year);
    fileInput.concat(",");
    fileInput.concat(hour);
    fileInput.concat(",");
    fileInput.concat(minute);
    fileInput.concat(",");
    fileInput.concat(seconds);
    fileInput.concat(",");
    fileInput.concat(millis());
    fileInput.concat(",");
    fileInput.concat("2.3");

    myFile.println(fileInput);
    Serial.print(fileInput);
    //myFile.flush();
  }


   if (millis() - timer > 5000){
    Serial.println("5s log");

    /* Display the results (acceleration is measured in m/s^2) 
    Serial.print("X: "); 
    Serial.print(acc_X); 
    Serial.print("  ");
    Serial.print("Y: "); 
    Serial.print(acc_Y); 
    Serial.print("  ");
    Serial.print("Z: "); 
    Serial.print(acc_Z); 
    Serial.print("  ");
    Serial.println("m/s^2 ");
    */


    stamp++;
    fileInput = "";
    fileInput.concat(stamp);
    fileInput.concat(",");
    myFile.print(fileInput);
    fileInput = "";


    fileInput.concat(acc_X);
    fileInput.concat(",");
    fileInput.concat(acc_Y);
    fileInput.concat(",");
    fileInput.concat(acc_Z);
    fileInput.concat(",");
    fileInput.concat(latitude);
    fileInput.concat(",");
    fileInput.concat(longitude);
    fileInput.concat(",");
    fileInput.concat(day);
    fileInput.concat(",");
    fileInput.concat(month);
    fileInput.concat(",");
    fileInput.concat(year);
    fileInput.concat(",");
    fileInput.concat(hour);
    fileInput.concat(",");
    fileInput.concat(minute);
    fileInput.concat(",");
    fileInput.concat(seconds);
    fileInput.concat(",");
    fileInput.concat(millis());
    fileInput.concat(",");
    fileInput.concat("3");

    myFile.println(fileInput);
    Serial.print(fileInput);
    //myFile.flush();

  myFile.flush();
}














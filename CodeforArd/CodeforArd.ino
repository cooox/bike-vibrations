//libaries 
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <elapsedMillis.h>

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
#define GPSECHO false

// Timers and timestamp
int stamp = 0;
uint32_t timer = millis();
elapsedMillis timer2;

// GPS variables
double latitude;
double longitude; 
int day;
int month; 
int year;
int hour; 
int minute;
int seconds;

// IMU variables
double acc_X;
double acc_Y;
double acc_Z;

// Our file on the SD card
File myFile;

// Pin configuration
const int vibsens_high1 = 6; //PIN empfindlicher Sensor, 18010, kurz
const int vibsens_high2 = 2;
const int vibsens_high3 = 0;
const int vibsens_low1 = 5;  //PIN weniger empfindlicher Sensor, 18030, lang
const int vibsens_low2 = 4;
const int vibsens_low3 = 3;

// change this to match your SD shield or module;
//     Adafruit SD shields and modules: pin 10
const int chipSelect = 10;

// Setup delay flag
boolean first = true;


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
}

String fileInputPreamble()
{
  /*
   * Every time the loop() detects a vibration or the timer
   * causes a write, all writes share a lot of values which
   * are collected in this function.
   *
   * Returns a String which needs the unique vibration type
   * identifier (1 to 7).
   */
  String fileInput = "";

  stamp++;
  fileInput.concat(stamp);
  fileInput.concat(",");
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

  return fileInput;
}

void setup() 
{
  Serial.begin(9600);
  Serial.print("Initializing SD card...");

  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(SS, OUTPUT);

  if (!SD.begin(chipSelect, 11, 12, 13)) {
    Serial.println("-initialization failed!");
    return;
  }
  Serial.println("+initialization done.");
  
  String data_file = "data_";
  data_file.concat(millis());
  char data_file_array[data_file.length()];
  data_file.toCharArray(data_file_array, data_file.length());
  
  //(re)create and open file to write in
  myFile = SD.open(data_file_array, FILE_WRITE);
  
  //GPS block--------------------------------------------------------------------
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out

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

  // Set up vibration sensors
  pinMode(vibsens_high1, INPUT_PULLUP);
  pinMode(vibsens_high2, INPUT_PULLUP);
  pinMode(vibsens_high3, INPUT_PULLUP);
  pinMode(vibsens_low1, INPUT_PULLUP);
  pinMode(vibsens_low2, INPUT_PULLUP);
  pinMode(vibsens_low3, INPUT_PULLUP);

  /* Initialise the sensor */
  if(!accel.begin()) {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }

  /* Display some basic information on this sensor */
  displaySensorDetails();
  
  timer2 = 0;

}


void loop() {
  /*
   * GPS readings and set up
   */
  char c = GPS.read();

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


  /*
   * Delay the first run for some time, so GPS gets ready
   */
  if (first) {
    // Wait for GPS to get ready
    first = false;
    delay(5000);
  }
  

  /*
   * Sensor readings and file input
   */
  sensors_event_t event; 
  accel.getEvent(&event);
  
  acc_X = event.acceleration.x;
  acc_Y = event.acceleration.y;
  acc_Z = event.acceleration.x;

  // approximately every minute, print out the current stats
  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis()) timer = millis();

  // Update GPS data approximately every 2 second
  if (millis() - timer > 2000) { 
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

  if (digitalRead(vibsens_high1) == LOW) {
    String fileInput = fileInputPreamble();
    Serial.println("schwache Vibration1");
    fileInput.concat(",1");
    
    myFile.println(fileInput);
    Serial.print(fileInput);
  }
  
  if (digitalRead(vibsens_high2) == LOW){
    String fileInput = fileInputPreamble();
    Serial.println("schwache Vibration2");
    fileInput.concat(",2");
    
    myFile.println(fileInput);
    Serial.print(fileInput);
  }
  
  if (digitalRead(vibsens_high3) == LOW){
    String fileInput = fileInputPreamble();
    Serial.println("schwache Vibration3");
    fileInput.concat(",3");
    
    myFile.println(fileInput);
    Serial.print(fileInput);
  }
  
  if (digitalRead(vibsens_low1) == LOW){
    String fileInput = fileInputPreamble();
    Serial.println("starke Vibration1");
    fileInput.concat(",4");
    
    myFile.println(fileInput);
    Serial.print(fileInput);
  }
  
  if (digitalRead(vibsens_low2) == LOW){
    String fileInput = fileInputPreamble();
    Serial.println("starke Vibration2");
    fileInput.concat(",5");
    
    myFile.println(fileInput);
    Serial.print(fileInput);
  }
  
  if (digitalRead(vibsens_low3) == LOW){
    String fileInput = fileInputPreamble();
    Serial.println("starke Vibration3");
    fileInput.concat(",6");
    
    myFile.println(fileInput);
    Serial.print(fileInput);
  }

  // Timer every 5 seconds
  if (timer2 > 5000) {
    timer2 = 0;

    String fileInput = fileInputPreamble();
    fileInput.concat(",7");
    
    myFile.println(fileInput);
    Serial.print(fileInput);
    Serial.println(", 5s log");
  }
  
  myFile.flush();
}


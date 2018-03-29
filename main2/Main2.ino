//Libraries for the pressure, altitude and temperature sensor
#include <Adafruit_BMP280.h>
//#include <Adafruit_Sensor.h>
#include <Wire.h>

//Library for the servo
#include <Servo.h>

//Library for the SD card reader
#include <SD.h>

//Libraries for the GPS module on the LORA/GPS shield
#include <NMEAGPS.h>
#include <GPSport.h>

//Library for managing the SPI pins of the leonardo
#include <SPI.h>

//Library for the LORA module on the LORA/GPS shield
#include <LoRa.h>

NMEAGPS  gps; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values

#define       SDChipS  7              //Chip select for the SD cacrd reader
#define       LoRaChipS  10           //Chip select for the LORA/GPS shield
#define       BMPChipS  6
int           fall = 0;               //Fall condition
int           temp, alt, altLast;     //Temperature, pressure and altitude
float         Pa;                     //Pressure
int           Time = 0;               //Times for calculating speed
int           lastTime;          
int           Speed;                  //How fast it's going (m/s)       

//Setting the Chip select of the pressure sensor to the module
Adafruit_BMP280 bmp(BMPChipS); // Using hardware SPI

static void digitalSwitch(int S, int B, int L){
  digitalWrite(SDChipS, S);
  digitalWrite(BMPChipS, B);
  digitalWrite(LoRaChipS, L);
  
  }

void setup()
{
  DEBUG_PORT.begin(9600);
  bmp.begin();

  pinMode(LoRaChipS, OUTPUT);
  pinMode(SDChipS, OUTPUT);
  pinMode(BMPChipS, OUTPUT);

  digitalSwitch(1,1,0);

  gpsPort.begin(9600);

  //Initializing the LORA and checking for success
  if (!LoRa.begin(915E6)) {
    Serial.println("LORA fail");
    //Don't do anything more:
    while(1);
  }
  
  //Serial.println("LoRa started");

  //Setting the SD card reader output to LOW and all others to HIGH
  digitalSwitch(0,1,1);

  //Serial.print("Initializing SD card...");

  //Initializing the SD card reader and checking for success
  if (!SD.begin(SDChipS)) {
    Serial.print("Card fail");
    //Don't do anything more:
    while(1);
  }

  digitalSwitch(1,0,1);

  //Serial.print("Initializing BMP280 sensor...");

  //Initializing the pressure sensor and checking for success
  if (!bmp.begin()) {  
    Serial.println(F("BMP fail"));
    while(1);
  }
}

//--------------------------


void loop()
{
  int timer = millis();
  
  lastTime = Time;
  Time = millis()/1000;

  digitalSwitch(1,1,0);
  
  while (gps.available( gpsPort )) {
    fix = gps.read();

    DEBUG_PORT.print( F("Location: ") );
    if (fix.valid.location) {
      DEBUG_PORT.print( fix.latitudeL());
      DEBUG_PORT.print( ',' );
      DEBUG_PORT.print( fix.longitudeL());
    }
  
    

    DEBUG_PORT.println();

   LoRa.beginPacket();
   LoRa.println(fix.longitudeL());
   LoRa.println(fix.latitudeL());
   LoRa.println(alt);
   //End the transmission to the reciever 
   LoRa.endPacket();
  
    //Switching modules to the pressure sensor for usage
    digitalSwitch(1,0,1);

    //Read temperature
    temp = bmp.readTemperature();
    //Serial.println(bmp.readTemperature());

    //Read Altitude
    altLast = alt;
    alt = bmp.readAltitude(1013); //Altitude property should be set to estimate current altitude
    //Serial.println(bmp.readAltitude(1183));
    
    //Read Atmospheric pressure
    Pa = bmp.readPressure();
    //Serial.println(bmp.readPressure());

     //Calculating estimate rise or fall speed
    Speed = ((alt-altLast)*100)/(Time-lastTime); //Meters per second, with two decimal point
    

    digitalSwitch(0,1,1);

    //Declaring variables from the SD card reader Library
    File data;

    //Writing a txt file for use (if the file doesn't exist, it will create it.)
    data = SD.open("data.txt", FILE_WRITE);

    //If the file is a valid one, save all information collected above
    if(data){
      Serial.println("Writing");
      data.print(fix.latitudeL());
      data.print(",");
      data.print(fix.longitudeL());
      data.print(alt);
      data.print(" m");
      data.println();
      data.print(temp);
      data.print(" *C,");
      data.print(Pa);
      data.print("Pa,");
      data.print(abs(Speed));
      data.print(" m/s,");
      data.print(Time);
      data.print(" seconds");
      data.println();
      data.close();
    }
  }

  //while((timer+1000)>millis());
    
  
if (fall == 0){
    //If the altitude is greater than 1000 meters
    if(alt >= 1000){
      drop();
      return;
    }

//  //If the longitude or latitude is past a certain area
//    if((lat <= -1)|| lon <= -1){
//      drop();
//    }
//
    //If a set amount of time has passed
    if(Time >= 10){ //ex. 600 = ten  minutes      
      drop();
    }

    //If the temperature drops below -20 degrees Celsius
    if(temp <= -20){
      drop();
    }
  }
}

static void drop(){

    int timer;
    //Creating servo object for the release
    Servo releaser;

    releaser.attach(5); //setting servo to pin 5
    
    //Release helium balloon
    for (int pos = 180; pos >= 75; pos -= 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    releaser.write(pos);              // tell servo to go to position in variable 'pos'
    timer = millis();
    while((timer+15)>millis());                       // waits 15ms for the servo to reach the position
    }

    fall = 1;

    
     return;
     //Release chute

     //return to the loop
  
  }

//Libraries
#include <DHT.h>;
#include <Wire.h>
#include <BH1750.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//Constants
const int chipSelect = 10;
#define SoilMoisturePin A0                      //Connect Soil Moisture Sensor Data pin to Arduino Analog pin 0
#define ONE_WIRE_BUS 2
#define DHTPIN 9                          //Connect DHT-22 Data Pin to Arduino Digital Pin 9
#define DHTTYPE DHT22                     // Define DHT Type 22
DHT dht(DHTPIN, DHTTYPE);                 // Initialize DHT sensor for normal 16mhz Arduino
BH1750 lightMeter;
RTC_DS3231 rtc;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//------Global Variables-------//
String DataStream;
int signalLED = 7;
int Interval = 5;                    //Data Acquistion Interval. Change this value if you want to change. Interval is in minutes.
//------Global Variables-------//

//------DHT 22-------//
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
//------DHT 22-------//

//------BH1750-------//
float lux;
//------BH1750-------//

//------DS3231-------//
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int minTime,secTime;
String Day, Time;
int dispCount = 0;
//------DS3231-------//

//------Soil Moisture Sensor-------//
 int soilmoistureReading = 0; 
//------Soil Moisture Sensor-------//

//------DS18B20-------//
float soilTemp;
//------DS18B20-------//

void setup() {
  Serial.begin(9600);
  dht.begin();
  Wire.begin();
  pinMode(signalLED, OUTPUT);
  sensors.begin();
  lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  else Serial.println("RTC Module initialized . . .");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
    digitalWrite(signalLED, LOW);
  }
  Serial.println("Micro SD Card initialized . . .");
  digitalWrite(signalLED, HIGH);
  Serial.print("Data Acquisition Interval: ");
  Serial.print(Interval);
  Serial.println(" minutes");
}

void loop() {
  getTime();
  if((minTime % Interval == 0) && secTime == 00){
    if(dispCount == 0){
        Serial.println();
        Serial.print("Day: ");
        Serial.print(Day);
        Serial.print(" Time: ");
        Serial.println(Time);
        getDHT22();
        getLightIntensity();
        getSoilMoisture();
        getSoilTemp();
        dispCount = 1;
        DataStream = Day + "," + Time + "," + hum + "," + temp + "," + lux + "," + soilmoistureReading + "," + soilTemp ;
        File dataFile = SD.open("datalog1.txt", FILE_WRITE);
        if (dataFile) {
          dataFile.println(DataStream);
          dataFile.close();
          // print to the serial port too:
          Serial.print("Data Sent to SD Card: ");
          Serial.println(DataStream);
          }
          // if the file isn't open, pop up an error:
         else {
          Serial.println("error opening datalog.txt");
         }   
    }
  }
  else {
    dispCount = 0;
  } 
}

void getDHT22(){
  hum = dht.readHumidity();
  temp= dht.readTemperature();
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");
  Serial.print("Air Temperature: ");
  Serial.print(temp);
  Serial.println(" Celsius");
}

void getLightIntensity(){
  lux = lightMeter.readLightLevel(true);
  Serial.print(F("Light Intensity: "));
  Serial.print(lux);
  Serial.println(F(" lux"));
  if (lux < 0) {
    Serial.println(F("Error condition detected"));
  }
  else {
    if (lux > 40000.0) {
      // reduce measurement time - needed in direct sun light
      if (lightMeter.setMTreg(32)) {
      }
      else {
        Serial.println(F("Error setting MTReg to low value for high light environment"));
      }
    }
    else {
        if (lux > 10.0) {
          // typical light environment
          if (lightMeter.setMTreg(69)) {
          }
          else {
            Serial.println(F("Error setting MTReg to default value for normal light environment"));
          }
        }
        else {
          if (lux <= 10.0) {
            //very low light environment
            if (lightMeter.setMTreg(138)) {
            }
            else {
              Serial.println(F("Error setting MTReg to high value for low light environment"));
            }
          }
       }
    }
  }
}

void getTime(){
  DateTime now = rtc.now();
  Day = String(now.month()) + "/" + String(now.day()) + "/" + String(now.year());
  if(now.minute()< 10)
    {
      if(now.second()< 10)
        {
         Time = String(now.hour()) + ":" + String("0") + String(now.minute()) + ":" + String("0") + String(now.second()); 
        }
      else
        {
         Time = String(now.hour()) + ":" + String("0") + String(now.minute()) + ":" + String(now.second());  
        }
    }
  else
    {
      if(now.second()< 10)
          {
           Time = String(now.hour()) + ":" + String(now.minute()) + ":" + String("0") + String(now.second()); 
          }
       else
      {
        Time = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
      }
    }
  
  minTime = now.minute();
  secTime = now.second();
  
}

void getSoilMoisture(){
  for (int i = 1; i <= 10; i++)                //Gets 10 Soil Moisture values and returns and Average Soil Moisture Reading
  { 
    soilmoistureReading = soilmoistureReading + analogRead(SoilMoisturePin); 
    delay(1); 
  } 
  soilmoistureReading = soilmoistureReading/10;
  soilmoistureReading = map(soilmoistureReading,0,1023,100,0);
  Serial.print("Soil Moisture: ");
  Serial.print(soilmoistureReading); 
  Serial.println(" %");
}

void getSoilTemp(){
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.print("Soil Temperature: ");
  soilTemp = sensors.getTempCByIndex(0);
  Serial.print(soilTemp);
  Serial.println(" Celsius");
}

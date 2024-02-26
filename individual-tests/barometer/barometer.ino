#include <Wire.h>
#include "SPI.h" //Why? Because library supports SPI and I2C connection
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP280.h"

//Setup connection of the sensor
Adafruit_BMP280 bmp; // I2C
/*//For SPI connection!
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10
//Adafruit_BMP280 bme(BMP_CS); // hardware SPI
//Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);
*/

//Variables
float pressure;		//To store the barometric pressure (Pa)
float temperature;	//To store the temperature (oC)
int altimeter; 		//To store the altimeter (m) (you can also use it as a float variable)

void setup() {
	bmp.begin();		//Begin the sensor
  	Serial.begin(9600);	//Begin serial communication at 9600bps
  	Serial.println("Adafruit BMP280 test:");
}

void loop() {
	//Read values from the sensor:
	pressure = bmp.readPressure();
	temperature = bmp.readTemperature();
	altimeter = bmp.readAltitude (1050.35); //Change the "1050.35" to your city current barrometric pressure (https://www.wunderground.com)
	
	//Print values to serial monitor:
  Serial.print(pressure);
  Serial.print(",");
  Serial.print(temperature);
  Serial.print(",");
  Serial.println(altimeter);
  

    
    delay(3000);} //Update every 5 sec
/* ------------------------ LIBRARIES ------------------------- */

// General
#include <Wire.h>
#include "SPI.h"
  #include <TimeLib.h>

// MicroSD [OV2640]
#include "FS.h"
#include "SD_MMC.h"

// Barometer [BMP280]
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP280.h"

/* ------------------- CONSTANTS/VARIABLES -------------------- */

String stream;

// MicroSD

// Barometer
Adafruit_BMP280 bmp;
float pressure;
float temperature;
int altitude;

/* -------------------- AUXILIAR FUNCTIONS -------------------- */

/* saveToFile - Write or appends data to a file.
 * @param path: The path of the file
 * @param data: The data to write or append
 * @return void
 */
void saveToFile(String path, String data, char mode) {
  File file;

  if (mode == 'a') {
    file = SD_MMC.open(path, FILE_APPEND);
  } else {
    file = SD_MMC.open(path, FILE_WRITE);
  }

  if (file) {
    file.println(data);
    file.close();
    Serial.println("Data appended/written to file successfully!");
  } else {
    Serial.println("Failed to open file for writing/appending");
  }
}

/* --------------------- MEASURE FUNCTIONS --------------------- */

void readBarometer() {
    pressure = bmp.readPressure();
    temperature = bmp.readTemperature();
    altitude = bmp.readAltitude(1013.25);

    stream = String(hour()) + ":" + String(minute()) + ":" + String(second()) + "," + String(pressure, 2) + "," + String(temperature, 2) + "," + String(altitude, 2);
    saveToFile("/test.txt", stream, 'a');
}

/* -------------------------- SETUP --------------------------- */

void setup() {
  Serial.begin(115200);
  setTime(0);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  // MicroSD
  Serial.print("Initializing the MicroSD card module... ");
  if (!SD_MMC.begin()) {
    Serial.println("MicroSD Card Mount Failed");
    return;
  }
  Serial.println("MicroSD OK!");

  String filename = "/data.txt";
  String data = "Sample data world";
  char mode = 'a';
  saveToFile(filename,data,mode);



  // Barometer
	bmp.begin();
}

void loop() {
  readBarometer();
  
  delay(1000);
}
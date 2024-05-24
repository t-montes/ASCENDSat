#include "TinyGPSPlus.h"
#include "SoftwareSerial.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

#define SEP ";"
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

SoftwareSerial serial_connection(16,17); // rxPin, txPin
TinyGPSPlus gps;
Adafruit_BMP280 bmp;

unsigned long now = 0;
unsigned long lastConnectionTime = 0;
const unsigned long connectionInterval = 1000;
int i = 0;
String data;

String accx = "0.0", accy = "0.0", accz = "0.0", roll = "0.0", yaw = "0.0", pitch = "0.0"; // accelerometer
String rollg = "0.0", yawg = "0.0", pitchg = "0.0"; // magnetometer
String pressure = "0.0", altitude = "0.0", temperature = "0.0"; // barometer
String altitudegps = "0.0", lat = "0.0", lon = "0.0"; // gps
String camera = "0.0", vbat = "0.0", vpv = "0.0"; // own

void setup() {
  Serial.begin(9600);
  serial_connection.begin(9600);
  Serial.println("GPS Start");

  Serial.println(F("BMP280 test"));
  unsigned status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
    while (1) delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void barometerRead() {
  temperature = String(bmp.readTemperature());
  float pressureValue = bmp.readPressure();
  float new_pressure = 0.968 * pressureValue + 2287.2;
  pressure = String(new_pressure);
  
  float alt = bmp.readAltitude(1011.9);
  float new_alt = 1.48 * alt - 1071.9;
  altitude = String(new_alt);
}

void gpsRead() {
  i=0;
  altitudegps = String(gps.altitude.meters());
  lat = String(gps.location.lat(), 6);
  lon = String(gps.location.lng(), 6);
}

void loop() {
  now = millis();
  while (serial_connection.available())
    gps.encode(serial_connection.read());

  if (gps.location.isUpdated()) gpsRead();
  else {
    if (now - lastConnectionTime >= connectionInterval) {
      i++;
      if (i != 1) Serial.println("GPS connecting... (" + String(i) + ")");

      barometerRead();
      data = String(now) + SEP +
             // accelerometer
             // magnetometer
             temperature + SEP + pressure + SEP + altitude + SEP +
             altitudegps + SEP + lat + SEP + lon;
      Serial.println(data);

      lastConnectionTime = now;
    }
  }
}



#include "TinyGPSPlus.h"
#include "SoftwareSerial.h"

SoftwareSerial serial_connection(16,17); // rxPin, txPin
TinyGPSPlus gps;

unsigned long now = 0;
unsigned long lastConnectionTime = 0;
const unsigned long connectionInterval = 1000;
int i = 0;

String accx = "0.0", accy = "0.0", accz = "0.0", roll = "0.0", yaw = "0.0", pitch = "0.0"; // accelerometer
String rollg = "0.0", yawg = "0.0", pitchg = "0.0"; // magnetometer
String pressure = "0.0", altitude = "0.0", temperature = "0.0"; // barometer
String altitudegps = "0.0", lat = "0.0", lon = "0.0"; // gps
String camera = "0.0", vbat = "0.0", vpv = "0.0"; // own

void setup() {
  Serial.begin(9600);
  serial_connection.begin(9600);
  Serial.println("GPS Start");
}

void gpsRead() {
  i=0;
  altitudegps = String(gps.altitude.meters());
  lat = String(gps.location.lat(), 6);
  lon = String(gps.location.lng(), 6);

  Serial.println(altitudegps + ";" + lat + ";" + lon);
}

void loop() {
  now = millis();
  while (serial_connection.available()) {gps.encode(serial_connection.read());}

  if (gps.location.isUpdated()) {
    gpsRead();
  } else {
    if (now - lastConnectionTime >= connectionInterval) {
      i++;
      Serial.println("connecting... ("+String(i)+")");
      lastConnectionTime = now;
    }
  }
}



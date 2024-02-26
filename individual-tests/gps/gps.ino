#include "TinyGPSPlus.h"
#include "SoftwareSerial.h"

SoftwareSerial serial_connection_gps(16,17); // rxPin, txPin
TinyGPSPlus gps;

unsigned long lastConnectionTime = 0; // Variable to track the last connection time
const unsigned long connectionInterval = 1000; // Interval between connection checks in milliseconds
int i = 0;

void setup() {
  Serial.begin(9600);
  serial_connection_gps.begin(9600);
  Serial.println("GPS Start");
}

void loop() {
  while (serial_connection_gps.available()) {
    gps.encode(serial_connection_gps.read());
  }

  if (gps.location.isUpdated()) {
    i=0;
    Serial.print("Satellite: ");
    Serial.println(gps.satellites.value());
    Serial.print("Lat: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", Lon: ");
    Serial.println(gps.location.lng(), 6);
    Serial.print("Speed MPH: ");
    Serial.println(gps.speed.mph());
    Serial.print("Altitude meter: ");
    Serial.println(gps.altitude.meters());
    Serial.println("-------------------------");
  } else {
    if (millis() - lastConnectionTime >= connectionInterval) {
      i++;
      Serial.println("connecting... ("+String(i)+")");
      lastConnectionTime = millis(); // Update the last connection time
    }
  }
}



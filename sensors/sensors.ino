#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "TinyGPSPlus.h"
#include "SoftwareSerial.h"
#include <EEPROM.h>

// Pin definitions
#define SD_CS_PIN 5
#define RX_PIN 16
#define TX_PIN 17
#define EEPROM_ADDRESS 0 // EEPROM address to store the file counter

// GPS setup
SoftwareSerial serial_connection(RX_PIN, TX_PIN); // rxPin, txPin
TinyGPSPlus gps;

// Function declarations
void createDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void setupSD();
void setupGPS();
void readGPSData();
void logData();
int readFileCounter();
void incrementFileCounter();

// Global variables
unsigned long lastConnectionTime = 0; // Variable to track the last connection time
const unsigned long connectionInterval = 1000; // Interval between connection checks in milliseconds
int connectionAttempts = 0;
float latitude = 0.0, longitude = 0.0, altitude = 0.0;
String currentFileName; // Variable to store the current file name

void createDir(fs::FS &fs, const char * path) {
  if (fs.mkdir(path)) {
    //Serial.println("Dir created");
  } else {
    //Serial.println("mkdir failed");
  }
}

void readFile(fs::FS &fs, const char * path) {
  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    //Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    //Serial.println("File written");
  } else {
    //Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    //Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    //Serial.println("Message appended");
  } else {
    //Serial.println("Append failed");
  }
  file.close();
}

void setupSD() {
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);

  int fileCounter = readFileCounter();
  currentFileName = "/measures" + String(fileCounter) + ".csv";
  writeFile(SD, currentFileName.c_str(), "time;accx;accy;accz;roll;yaw;pitch;rollg;yawg;pitchg;pressure;altitud;altitudegps;lat;lon;camera;vbat;vpv\n");

  Serial.printf("Writing to %11u\n", currentFileName);

  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  incrementFileCounter();
}

void setupGPS() {
  serial_connection.begin(9600);
  Serial.println("GPS Start");
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512); // Initialize EEPROM
  setupSD();
  setupGPS();
}

void readGPSData() {
  connectionAttempts = 0;
  latitude = gps.location.lat();
  longitude = gps.location.lng();
  altitude = gps.altitude.meters();
  Serial.print("Altitude: ");
  Serial.println(altitude);
}

void logData() {
  // Get current time
  unsigned long currentTime = millis();

  // Placeholder values for other sensors
  float accx = 0.0, accy = 0.0, accz = 0.0;
  float roll = 0.0, yaw = 0.0, pitch = 0.0;
  float rollg = 0.0, yawg = 0.0, pitchg = 0.0;
  float pressure = 0.0, altitud = 0.0;
  float camera = 0.0, vbat = 0.0, vpv = 0.0;

  String dataString = String(currentTime) + ";" +
                      String(accx) + ";" + String(accy) + ";" + String(accz) + ";" +
                      String(roll) + ";" + String(yaw) + ";" + String(pitch) + ";" +
                      String(rollg) + ";" + String(yawg) + ";" + String(pitchg) + ";" +
                      String(pressure) + ";" + String(altitud) + ";" +
                      String(altitude, 2) + ";" + String(latitude, 6) + ";" +
                      String(longitude, 6) + ";" + String(camera) + ";" +
                      String(vbat) + ";" + String(vpv) + "\n";

  Serial.print(dataString);
  appendFile(SD, currentFileName.c_str(), dataString.c_str());
}

int readFileCounter() {
  return EEPROM.read(EEPROM_ADDRESS);
}

void incrementFileCounter() {
  int fileCounter = readFileCounter();
  fileCounter++;
  EEPROM.write(EEPROM_ADDRESS, fileCounter);
  EEPROM.commit();
}

void loop() {
  while (serial_connection.available()) {
    gps.encode(serial_connection.read());
  }
  if (gps.location.isUpdated()) {
    readGPSData();
  } else {
    if (millis() - lastConnectionTime >= connectionInterval) {
      connectionAttempts++;
      Serial.println("connecting... (" + String(connectionAttempts) + ")");
      lastConnectionTime = millis(); // Update the last connection time
    }
  }

  logData();

  delay(500);
}

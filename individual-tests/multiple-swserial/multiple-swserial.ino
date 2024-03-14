#include "TinyGPSPlus.h"
#include "SoftwareSerial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

SoftwareSerial serial_connection(16,17); // rxPin, txPin for the first GPS module
SoftwareSerial serial_connection_2(3, 1); // rxPin, txPin for the second GPS module
TinyGPSPlus gps;
TinyGPSPlus gps2;

unsigned long lastConnectionTime1 = 0; // Variable to track the last connection time for GPS module 1
unsigned long lastConnectionTime2 = 0; // Variable to track the last connection time for GPS module 2
const unsigned long connectionInterval = 1000; // Interval between connection checks in milliseconds
int i = 0; // Counter for GPS module 1
int j = 0; // Counter for GPS module 2

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char * path){
  Serial.printf("Removing Dir: %s\n", path);
  if(fs.rmdir(path)){
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}


void setup() {
  Serial.begin(9600);
  serial_connection.begin(9600);
  serial_connection_2.begin(9600);
  Serial.println("GPS Start");

  // An SD is required to debug (as the RX/TX for the USB Serial is 3,1)
  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  writeFile(SD, "/hello.txt", "Hola ");
  appendFile(SD, "/hello.txt", "Mundo!\n");
  readFile(SD, "/hello.txt");

  writeFile(SD, "/test-gps-1.txt", "Start 1...\n");
  writeFile(SD, "/test-gps-2.txt", "Start 2...\n");
}

void loop() {
  while (serial_connection.available()) {
    gps.encode(serial_connection.read());
  }
  
  while (serial_connection_2.available()) {
    gps2.encode(serial_connection_2.read());
  }
  
  if (gps.location.isUpdated()) {
    i=0;
    Serial.print("Satellite 1: ");
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

    appendFile(SD, "/test-gps-1.txt", "Satellite 1: ");
    appendFile(SD, "/test-gps-1.txt", String(gps.satellites.value()).c_str());
    appendFile(SD, "/test-gps-1.txt", "\nLat: ");
    appendFile(SD, "/test-gps-1.txt", String(gps.location.lat()).c_str());
    appendFile(SD, "/test-gps-1.txt", ", Lon: ");
    appendFile(SD, "/test-gps-1.txt", String(gps.location.lng()).c_str());
    appendFile(SD, "/test-gps-1.txt", "\nSpeed MPH: ");
    appendFile(SD, "/test-gps-1.txt", String(gps.speed.mph()).c_str());
    appendFile(SD, "/test-gps-1.txt", "\nAltitude meter: ");
    appendFile(SD, "/test-gps-1.txt", String(gps.altitude.meters()).c_str());
    appendFile(SD, "/test-gps-1.txt", "\n-------------------------\n");
  } else {
    if (millis() - lastConnectionTime1 >= connectionInterval) {
      i++;
      Serial.println("connecting 1... ("+String(i)+")");
      appendFile(SD, "/test-gps-1.txt", ("connecting 1... ("+String(i)+")\n").c_str());
      lastConnectionTime1 = millis();
    }
  }

  if (gps2.location.isUpdated()) {
    j=0;
    Serial.print("Satellite 2: ");
    Serial.println(gps2.satellites.value());
    Serial.print("Lat: ");
    Serial.print(gps2.location.lat(), 6);
    Serial.print(", Lon: ");
    Serial.println(gps2.location.lng(), 6);
    Serial.print("Speed MPH: ");
    Serial.println(gps2.speed.mph());
    Serial.print("Altitude meter: ");
    Serial.println(gps2.altitude.meters());
    Serial.println("-------------------------");

    appendFile(SD, "/test-gps-2.txt", "Satellite 2: ");
    appendFile(SD, "/test-gps-2.txt", String(gps.satellites.value()).c_str());
    appendFile(SD, "/test-gps-2.txt", "\nLat: ");
    appendFile(SD, "/test-gps-2.txt", String(gps.location.lat()).c_str());
    appendFile(SD, "/test-gps-2.txt", ", Lon: ");
    appendFile(SD, "/test-gps-2.txt", String(gps.location.lng()).c_str());
    appendFile(SD, "/test-gps-2.txt", "\nSpeed MPH: ");
    appendFile(SD, "/test-gps-2.txt", String(gps.speed.mph()).c_str());
    appendFile(SD, "/test-gps-2.txt", "\nAltitude meter: ");
    appendFile(SD, "/test-gps-2.txt", String(gps.altitude.meters()).c_str());
    appendFile(SD, "/test-gps-2.txt", "\n-------------------------\n");
  } else {
    if (millis() - lastConnectionTime2 >= connectionInterval) {
      j++;
      Serial.println("connecting 2... ("+String(j)+")");
      appendFile(SD, "/test-gps-2.txt", ("connecting 2... ("+String(j)+")\n").c_str());
      lastConnectionTime2 = millis();
    }
  }
}

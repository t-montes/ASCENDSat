#include "FS.h"
#include "SD.h"
#include "SPI.h"

void createDir(fs::FS &fs, const char * path){
  if(fs.mkdir(path)){
    //Serial.println("Dir created");
  } else {
    //Serial.println("mkdir failed");
  }
}

void readFile(fs::FS &fs, const char * path){
  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    //Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    //Serial.println("File written");
  } else {
    //Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_APPEND);
  if(!file){
    //Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
    //Serial.println("Message appended");
  } else {
    //Serial.println("Append failed");
  }
  file.close();
}

void testFileIO(fs::FS &fs, const char * path){
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if(file){
    len = file.size();
    size_t flen = len;
    start = millis();
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    //Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    //Serial.println("Failed to open file for reading");
  }

  file = fs.open(path, FILE_WRITE);
  if(!file){
    //Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  file.close();
}

void setup(){
  Serial.begin(115200);
  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  //Serial.printf("SD Card Size: %lluMB\n", cardSize);

  writeFile(SD, "/hello.txt", "Hello ");
  appendFile(SD, "/hello.txt", "World!\n");
  testFileIO(SD, "/test.txt");
  //Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  //Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void loop(){
  
}
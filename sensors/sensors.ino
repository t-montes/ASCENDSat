#include "TinyGPSPlus.h"
#include "SoftwareSerial.h"
#include <Wire.h>
#include <SPI.h>
#include <MechaQMC5883.h>
#include <Adafruit_BMP280.h>
#include "FS.h"
#include "SD.h"

#define SEP ";"
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

SoftwareSerial serial_connection(16,17); // rxPin, txPin
TinyGPSPlus gps;
Adafruit_BMP280 bmp;
MechaQMC5883 qmc;

unsigned long now = 0;
unsigned long lastConnectionTime = 0;
const unsigned long connectionInterval = 1000;
int i = 0;
String data;

float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
int RateCalibrationNumber;
int AccCalibrationNumber;
float AccX, AccY, AccZ;
float AccXCalibration, AccYCalibration, AccZCalibration;
float AngleRoll, AnglePitch, AngleYaw;
const int pinvolt= 34; // Pin analógico al que está conectado el sensor de luz
float vout;
float value;
float valor;
float altura_inicial;

int mx_bias=-287;
int my_bias=239;
int mz_bias=275;

String accx = "0.0", accy = "0.0", accz = "0.0", roll = "0.0", yaw = "0.0", pitch = "0.0"; // accelerometer
String rollg = "0.0", yawg = "0.0", pitchg = "0.0"; // magnetometer
String pressure = "0.0", altitude = "0.0", temperature = "0.0"; // barometer
String altitudegps = "0.0", lat = "0.0", lon = "0.0"; // gps
String camera = "0.0", vbat = "0.0", vpv = "0.0"; // own

void setup() {
  Serial.println("Init");
  Serial.begin(9600);
  serial_connection.begin(9600);
  Serial.println("GPS Start");

  Serial.println(F("BMP280 test"));
  unsigned status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or try a different address!"));
    //while (1) delay(10);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  Wire.begin();
  qmc.init();
  Wire.setClock(400000);
  Wire.begin();
  delay(250);
  Wire.beginTransmission(0x68); 
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  pinMode(pinvolt, INPUT);

  float altura_acumulada;
    for (int i = 0; i < 5; i++) {
      barometerRead();
      altura_inicial = float(new_alt);
      altura_acumulada += new_alt;
      delay(100);
    }
  altura_inicial = altura_inicial/5;

  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
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
  
  writeFile(SD, "/measures.csv", "# start\n");
}

void accelerometerRead(){
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);
  Wire.write(0x05);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(); 
  Wire.requestFrom(0x68,6);
  int16_t AccXLSB = Wire.read() << 8 | Wire.read();
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();
  Wire.beginTransmission(0x68);
  Wire.write(0x1B); 
  Wire.write(0x8);
  Wire.endTransmission();     
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  int16_t GyroX=Wire.read()<<8 | Wire.read();
  int16_t GyroY=Wire.read()<<8 | Wire.read();
  int16_t GyroZ=Wire.read()<<8 | Wire.read();
  RateRoll=(float)GyroX/65.5;
  RatePitch=(float)GyroY/65.5;
  RateYaw=(float)GyroZ/65.5;
  AccX=(float)AccXLSB/4096;//+0.01;
  AccY=(float)AccYLSB/4096;//-0.01;
  AccZ=(float)AccZLSB/4096;//-0.12;
  AngleRoll=atan(AccY/sqrt(AccX*AccX+AccZ*AccZ))*1/(3.142/180);
  AnglePitch=-atan(AccX/sqrt(AccY*AccY+AccZ*AccZ))*1/(3.142/180);
  roll = String(AngleRoll);
  pitch = String(AnglePitch);

  accx = String(AccX);
  accy = String(AccY);
  accz = String(AccZ);
}

void accelerometerCalibration(){
  for (RateCalibrationNumber=0; RateCalibrationNumber<2000; RateCalibrationNumber ++) {
    accelerometerRead();
    RateCalibrationRoll+=RateRoll;
    RateCalibrationPitch+=RatePitch;
    RateCalibrationYaw+=RateYaw;
    AccXCalibration+=AccX;
    AccYCalibration+=AccY;
    AccZCalibration+=AccZ;
    delay(1);
  }
  RateCalibrationRoll/=4000;
  RateCalibrationPitch/=4000;
  RateCalibrationYaw/=4000;
  AccXCalibration/=4000;
  AccYCalibration/=4000;
  AccZCalibration/=4000;
}

void magnetometerRead() {
  int x, y, z;
  qmc.read(&x, &y, &z);
  rollg = String(x);
  yawg = String(y);
  pitchg = String(z);

  AngleYaw = atan2(y,x)*(180/3.1415);
  yaw = String(AngleYaw);
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

void voltRead() {
 value = analogRead(pinvolt);
 valor = value*3.13/4095;
 vbat = String(0.4 + valor*(9872+1000)/9872);
}

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
    Serial.printf("%u bytes read for %u ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }


  file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for(i=0; i<2048; i++){
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
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
      magnetometerRead();
      accelerometerRead();
      voltRead();

      data = String(now) + SEP +
             accx + SEP + accy + SEP + accz + SEP + roll + SEP + yaw + SEP + pitch + SEP +
             rollg + SEP + yawg + SEP + pitchg + SEP +
             temperature + SEP + pressure + SEP + altitude + SEP +
             altitudegps + SEP + lat + SEP + lon + SEP + 
             camera + SEP + vbat + SEP + vpv;

      Serial.println(data);
      lastConnectionTime = now;
      data = data + "\n";
      appendFile(SD, "/measures.csv", data);
    }
  }

  if (abs(float(alt) - faltura_inicial) > 100) {
    // activar servo
    digitalWrite(pinOutput, HIGH);
  } else {
    digitalWrite(pinOutput, LOW);
  }
}

#include "TinyGPSPlus.h"
#include "SoftwareSerial.h"
#include <Wire.h>
#include <SPI.h>
#include <MechaQMC5883.h>
#include <Adafruit_BMP280.h>

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

  float altura_inicial
  float altura_acumulada
    for (int i = 0; i < 5; i++) {
      barometerRead();
      altura_inicial = float(new_alt);
      altura_acumulada += new_alt;
      delay(100);
    }
  altura_inicial = altura_inicial/5
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

      lastConnectionTime = now;
      Serial.println(data);
    }
  }

  if (abs(float(alt) - faltura_inicial) > 100) {
    // activar servo
    digitalWrite(pinOutput, HIGH);
  } else {
    digitalWrite(pinOutput, LOW);
  }
}

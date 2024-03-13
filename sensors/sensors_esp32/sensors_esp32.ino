#include <Wire.h>
#include <BMP280_DEV.h>
#include <MechaQMC5883.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "TinyGPSPlus.h"
#include "SoftwareSerial.h"

#define I2C_SDA 14 //21
#define I2C_SCL 15 //22
#define RX_PIN 7 //16
#define TX_PIN 6 //17

#define BMP280_ADDRESS 0x76
#define HMC5883L_ADDRESS 0x0D

BMP280_DEV bmp(I2C_SDA, I2C_SCL);
MechaQMC5883 qmc;
Adafruit_MPU6050 mpu;

float temperature, pressure, altitude;

SoftwareSerial serial_connection(16,17); // rxPin, txPin
TinyGPSPlus gps;
unsigned long lastConnectionTime = 0; // Variable to track the last connection time
const unsigned long connectionInterval = 1000; // Interval between connection checks in milliseconds
int i = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  // BMP280 setup
  Serial.println("Initializing BMP280...");
  if (!bmp.begin(BMP280_ADDRESS)) {
    Serial.println("Failed to find BMP280 sensor");
  }
  bmp.setTimeStandby(TIME_STANDBY_2000MS);
  bmp.startNormalConversion();

  // HMC5883L setup
  Serial.println("Initializing HMC5883L...");
  qmc.init();

  // MPU6050 setup
  Serial.println("Initializing MPU6050...");
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 sensor");
  }

  // GPS setup
  serial_connection.begin(9600);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("Initialization complete.");
  delay(100);
}

void loop() {
  while (serial_connection.available()) {
    gps.encode(serial_connection.read());
  }

  // BMP280 readings
  bmp.getMeasurements(temperature, pressure, altitude);
  Serial.print("BMP280 - Temperature: ");
  Serial.println(temperature);
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.print("Altitude: ");
  Serial.println(altitude);

  // HMC5883L readings
  int x, y, z;
  qmc.read(&x, &y, &z);
  Serial.print("HMC5883L - Mx: ");
  Serial.print(x);
  Serial.print(" My: ");
  Serial.print(y);
  Serial.print(" Mz: ");
  Serial.println(z);

  // MPU6050 readings
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  Serial.print("MPU6050 - Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  // GPS readings
  if (gps.location.isUpdated()) {
    i=0;
    Serial.print("GPS - Satellite: ");
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
      Serial.println("GPS Connecting... ("+String(i)+")");
      lastConnectionTime = millis(); // Update the last connection time
    }
  }

  Serial.println("");
  delay(500);
}

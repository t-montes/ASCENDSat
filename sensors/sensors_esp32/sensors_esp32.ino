#include <Wire.h>
#include <BMP280_DEV.h>

#define I2C_SDA 14
#define I2C_SCL 15
BMP280_DEV bmp(I2C_SDA, I2C_SCL);

float temperature, pressure, altitude;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.println("Testing BMP280...");
  if (!bmp.begin(0x76)) {
    Serial.print("Oops...something went wrong... please check your BMP280 sensor");
    //while (1);
  }
  bmp.setTimeStandby(TIME_STANDBY_2000MS);
  bmp.startNormalConversion();
}

void loop() {
  // BMP280 readings need to be on loop to get accurate readings
  bmp.getMeasurements(temperature, pressure, altitude);

  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.print("Altitude: ");
  Serial.println(altitude);
}

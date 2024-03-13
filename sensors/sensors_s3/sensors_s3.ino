/* ------------------------ LIBRARIES ------------------------- */

// General
#include <Wire.h>
#include "SPI.h"

// MicroSD and Camera [OV2640]
#include "esp_camera.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <TimeLib.h>
#define CAMERA_MODEL_XIAO_ESP32S3
#include "camera_pins.h"

// Barometer [BMP280]
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP280.h"

/* ------------------- CONSTANTS/VARIABLES -------------------- */

// MicroSD and Camera
unsigned long lastCaptureTime = 0;
int imageCount = 1;
bool cameraStatus = false;
bool sdStatus = false;

// Barometer
Adafruit_BMP280 bmp;
float pressure = 0;
float temperature = 0;
int altitude = 0;

/* -------------------- AUXILIAR FUNCTIONS -------------------- */

/*
 * savePhoto - Take a photo and save it to a file.
 * @param fileName The name of the file
 * @returns void
 */
void savePhoto(const char * fileName) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to get camera frame buffer");
    return;
  }
  writeImage(SD, fileName, fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
  Serial.printf("Saved picture: %s\r\n", fileName);
}

/*
 * writeImage - Saves image contents to a file.
 * @param fs The filesystem
 * @param path The path to the file
 * @param data The image data
 * @param len The length of the image (to check for errors)
 */
void writeImage(fs::FS &fs, const char * path, uint8_t * data, size_t len) {
    Serial.printf("Writing image to: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.write(data, len) == len){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

/*
 * appendData - Appends data to a file (used for sensors data).
 * @param path The path to the file
 * @param data The data to append
 * @returns void
 */
void appendData(const char * path, const char * data) {
  File file = SD.open(path, FILE_APPEND);
  if (file) {
    file.println(data);
    file.close();
    Serial.println("Data appended to file");
  } else {
    Serial.println("Failed to open file for appending");
  }
}

/* --------------------- MEASURE FUNCTIONS --------------------- */

void readBarometer() {
    pressure = bmp.readPressure();
    temperature = bmp.readTemperature();
    altitude = bmp.readAltitude(1013.25);
}

/* -------------------------- SETUP --------------------------- */

void setup() {
  Serial.begin(115200);
  while(!Serial);

  // MicroSD and Camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
    #if CONFIG_IDF_TARGET_ESP32S3
        config.fb_count = 2;
    #endif
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  cameraStatus = true;

  if(!SD.begin(21)){
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

  sdStatus = true;

  setTime(0);

  // Barometer
	bmp.begin();
}

/* --------------------------- LOOP --------------------------- */

void loop() {
  Serial.println("0000000000000000000");
  Serial.println(pressure);
  Serial.println(temperature);
  Serial.println(altitude);

  // 1. Reading stage
  readBarometer();

  Serial.println("1111111111111111111");
  Serial.println(pressure);
  Serial.println(temperature);
  Serial.println(altitude);

  // 2. Saving stage
  if(cameraStatus && sdStatus){
    unsigned long now = millis();
  
    // 2.1. Picture (only for testing)
    if ((now - lastCaptureTime) >= 5000) {
      char filename[32];
      sprintf(filename, "/image%d.jpg", imageCount);
      savePhoto(filename);

      imageCount++;
      lastCaptureTime = now;
    }

    // 2.2. Formatting
    char dataString[50];
    sprintf(dataString, "%02d:%02d:%02d,%.2f,%.2f,%.2f\r\n", hour(), minute(), second(), pressure, temperature, altitude);
    Serial.print("Data: ");
    Serial.println(dataString);

    // 2.3. Appending to SD file
    appendData("/data.txt", dataString);
  }
}
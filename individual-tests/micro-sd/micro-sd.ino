// MicroSD Libraries
#include "FS.h"
#include "SD_MMC.h"

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);

  // Initialize the MicroSD
  Serial.print("Initializing the MicroSD card module... ");
  if (!SD_MMC.begin()) {
    Serial.println("MicroSD Card Mount Failed");
    return;
  }
  Serial.println("MicroSD OK!");

  // Write data to file
  File file = SD_MMC.open("/hello.txt", FILE_WRITE);
  if (file) {
    file.println("Hello MicroSD!");
    file.close();
    Serial.println("File 'hello.txt' created and written successfully!");
  } else {
    Serial.println("Failed to open file 'hello.txt' for writing");
  }
}

void loop() {
  // Nothing to do in the loop for this application
}

// MicroSD Libraries
#include "FS.h"
#include "SD_MMC.h"

void save_to_file(String filename, String data, char mode) {
  File file;

  if (mode == 'a') {
    file = SD_MMC.open(filename, FILE_APPEND);
  } else {
    file = SD_MMC.open(filename, FILE_WRITE);
  }

  if (file) {
    file.println(data);
    file.close();
    Serial.println("Data appended/written to file successfully!");
  } else {
    Serial.println("Failed to open file for writing/appending");
  }
}

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

  String filename = "/data.txt"; // Specify the filename
  String data = "This is some data to be saved to the file."; // Data to be saved
  char mode = 'a'; // Specify the mode ('a' for append or 'w' for write)

  save_to_file(filename,data,mode);
}

void loop() {
  // Nothing to do in the loop for this application
}

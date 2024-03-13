#include <Wire.h>
#include <MechaQMC5883.h>

MechaQMC5883 qmc;

void setup() {
    Wire.begin();
    Serial.begin(9600);
    qmc.init();
}

void loop() {
    int x,y,z;
    qmc.read(&x,&y,&z);

    Serial.print("x: ");
    Serial.print(x);
    Serial.print(" y: ");
    Serial.print(y);
    Serial.print(" z: ");
    Serial.print(z);
    Serial.println();
    delay(100);
}
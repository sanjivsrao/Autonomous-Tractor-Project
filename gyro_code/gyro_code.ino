#include <SoftwareSerial.h>
#include <MPU6050_light.h>
#include "Wire.h"
#define sdaPin 4
#define sclPin 5

unsigned long timer = 0;
// Set up a new SoftwareSerial object
SoftwareSerial mySerial =  SoftwareSerial(sdaPin, sclPin);
MPU6050 mpu(Wire);

void setup()  {
    mpu.begin();
    delay(1000);
    mpu.calcOffsets();
    // Set the baud rate for the SoftwareSerial object
    mySerial.begin(9600);
}

void loop() {
  mpu.update();

  if((millis()-timer)>10){ // print data every 10ms
    Serial.print("X : ");
    Serial.print(mpu.getAngleX());
    Serial.print("\tY : ");
    Serial.print(mpu.getAngleY());
    Serial.print("\tZ : ");
    Serial.println(mpu.getAngleZ());
    timer = millis();
  }

    if (mySerial.available() > 0) {
        Serial.print (mySerial.read());
    }
}

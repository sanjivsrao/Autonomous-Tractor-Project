#include <SoftwareSerial.h>
#include <MPU6050_light.h>
#include "Wire.h"
#define rxPin 2
#define txPin 3

unsigned long timer = 0;
// Set up a new SoftwareSerial object
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);
MPU6050 mpu(Wire);

void setup()  {
    // Define pin modes for TX and RX
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);
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
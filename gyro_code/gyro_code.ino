#include <SoftwareSerial.h>
#include <MPU6050_light.h>
#include "Wire.h"

//test
#define accumulatedMillis millis() - timerMillis

//communication pins
#define rxPin 2
#define txPin 3

//push button pin
#define buttonPin 12

//left motor pins
#define enA 11
#define in1 7
#define in2 6
//right motor pins
#define enB 10
#define in3 5
#define in4 4
//bluetooth serial
SoftwareSerial mySerial(8,9);

// Set up a new SoftwareSerial object
MPU6050 mpu(Wire);

const unsigned long motorTimerPreset = 2000;  // two seconds
unsigned long timerMillis;  // For counting time increments
unsigned long timer = 0;
int buttonState;
bool switcher = false;

// states the tractor could be in
enum {OFF, MOVE, TURN90, TURN180};
unsigned char currentState;  // tractor state at any given moment
// bluetooth char
  String cmd;
// gyroscope float
float z;
float z_init;

void setup() {
  Serial.begin(115200);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  //button
  pinMode(buttonPin, INPUT);
  //left motor
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  //init direction for left motor
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  //right motor
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  //init direction for right motor
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  //gyro setup
  Wire.begin();
  mpu.begin();
  mpu.calcOffsets();
  mySerial.begin(9600);
  Serial.begin(9600);
 //init state
 currentState = OFF;
}

void loop() {
  mpu.update();
  if((millis()-timer)>1000) 
  {                                                                
    
    Serial.print("Yaw: ");
    float z = mpu.getAngleZ();
    Serial.print(mpu.getAngleZ());
    if (z > 0) {
      Serial.print("yeet");
      analogWrite(enA, 200);
          analogWrite(enB, 180);
      
    }
    Serial.println("\n");
    timer = millis();  
  }
}

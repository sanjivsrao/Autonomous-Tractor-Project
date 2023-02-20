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
SoftwareSerial mySerial(8, 9);

// Set up a new SoftwareSerial object
MPU6050 mpu(Wire);

const unsigned long motorTimerPreset = 2000;  // two seconds
unsigned long timerMillis;  // For counting time increments

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
  z_init = mpu.getAngleZ();
  Serial.begin(9600);
  //init state
  currentState = OFF;
}

void loop() {
  mpu.update();
  if (mySerial.available()) {
    cmd = mySerial.readString();
    if (cmd == "off") {
      mySerial.println("Turning off Robot");
    }
    else if (cmd == "on") {
      mySerial.println("Turning on Robot");
    }
    else {
      mySerial.println("Invalid command");
    }
  }

  buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);
  if (buttonState == 1) {
    switcher = true;
  }
  delay (250);
  //debounce button
  switch (currentState) {
    case OFF: // Nothing happening, waiting for switchInput
      analogWrite(enA, 0);
      analogWrite(enB, 0);
      Serial.println("OFF");
      if (cmd == "on" || switcher) {
        switcher = false;
        currentState = MOVE;
        break;
      }
      else {
        currentState = OFF;
        break;
      }

    case MOVE:
      Serial.println("Moving!!!!!");
      analogWrite(enA, 200);
      analogWrite(enB, 200);
      if (cmd == "off" || switcher) {
        switcher = false;
        currentState = OFF;
        timerMillis = millis();
        break;
      }
      z = mpu.getAngleZ();
      Serial.println(z);
      if ((millis() - timerMillis) > 1000) {
        if (z > 0) {
          analogWrite(enA, 100);
          analogWrite(enB, 140);
          currentState = MOVE;
          delay(1000);
          break;
        }
        if (z < 0) {
          analogWrite(enA, 140);
          analogWrite(enB, 100);
          currentState = MOVE;
          delay(1000);
          break;
        }
        timerMillis = millis();
      }
      break;

      //      digitalWrite(openLED, motorRun);
      //
      // The compare below would be replaced by a test of a limit
      // switch, or other sensor, in a real application.
      //      if (accumulatedMillis >= motorTimerPreset) { // Door up
      //        digitalWrite( openLED, motorStop); // Stop the motor
      ////        doorState = doorIsUp; // The door is now open
      break;


    case TURN90:
      Serial.println("door up");
    //      if (digitalRead(switchInput) == LOW) { // switchInput pressed
    //        timerMillis = millis(); // reset the timer
    ////        doorState = doorClosing; // Advance to the next state
    //        break;
    //      }
    //      else { // switchInput was not pressed
    //        break;
    //      }

    case TURN180:
      Serial.println("door closing");

    default:
      Serial.println("\n We hit the default");
      if (mySerial.available() > 0) {
        Serial.write(mySerial.read());
      }
      break;
  }
  // Clears the command
  cmd = "";
  buttonState = 0;
}

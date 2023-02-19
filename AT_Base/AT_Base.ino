#include <SoftwareSerial.h>
#include <MPU6050_light.h>
#include "Wire.h"


#define accumulatedMillis millis() - timerMillis

//communication pins
#define rxPin 2
#define txPin 3

//push button pin
#define buttonPin 12

//motor pins
#define enA 11
#define in1 7
#define in2 6

//bluetooth serial
SoftwareSerial mySerial(8,9);

// Set up a new SoftwareSerial object
MPU6050 mpu(Wire);

const unsigned long motorTimerPreset = 2000;  // two seconds
unsigned long timerMillis;  // For counting time increments

int buttonState = 0;

// states the tractor could be in
enum {OFF, MOVE, TURN90, TURN180};
unsigned char currentState;  // tractor state at any given moment

void setup() {
  Serial.begin(115200);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  mpu.begin();
  mpu.calcOffsets();
  mySerial.begin(9600);
  Serial.begin(9600);
  currentState = OFF;
  //sets init direction for motos
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  
}

void loop() {
  mpu.update();
  buttonState = digitalRead(buttonPin);
  switch (currentState) {
    case OFF: // Nothing happening, waiting for switchInput
      if (mySerial.available()>0) {
       char bt = mySerial.read();
        if (bt == 'o') {
          currentState = MOVE;
          break;
        }  
      }
      else if (buttonState == HIGH) {
        currentState = MOVE;
        break;
      }
      else {
        currentState = OFF;
        break;
      }
      
    case MOVE:
      Serial.println("Moving!!!!!");
      analogWrite(enA, 100);
       if (buttonState == LOW) {
        currentState = OFF;
        break;
      }
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
      if (mySerial.available()>0) {
        Serial.write(mySerial.read());
      }
      break;
  }
}

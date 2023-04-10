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

unsigned long timer = 0;

// states the tractor could be in
enum {OFF, MOVE, TURN90, TURN180};
unsigned char currentState;  // tractor state at any given moment

// states the button can be in
enum {PUSHED, RELEASED};

unsigned char buttonState; // button state at any given moment
bool buttonCommand; // boolean conversion from button input
int buttonRead; // command boolean used for directing tractor FSM logic
unsigned long debounceDelay = 50;
unsigned long debounceTime = 0;


// bluetooth char
String cmd;
// gyroscope float
float z;
float z_init;

void setup() {
  Serial.begin(9600);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  //button
  pinMode(buttonPin, INPUT);
  //left motor
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  //init direction for left motor
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  //right motor
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  //init direction for right motor
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  
  //gyro setup
  Wire.begin();
  mpu.begin();
  mpu.calcGyroOffsets();
  mySerial.begin(9600);
  z_init = mpu.getAngleZ();

  //init state
  currentState = OFF;
  buttonState = RELEASED;
  buttonCommand = false;
}

void loop() {
  mpu.update();
  float x = mpu.getAngleX();
  float y = mpu.getAngleY();
  float z = -(mpu.getAngleZ());
  
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
  
  //debounce button
  switch (buttonState) {
    case PUSHED:
      buttonRead = digitalRead(buttonPin);
      if (!buttonRead){
        buttonState=RELEASED;
        debounceTime = millis();
      }   
      break;
    case RELEASED:
      buttonRead = digitalRead(buttonPin);
      if (buttonRead){
        buttonState = PUSHED;
        buttonCommand = true; 
      }
      break;
  }


  switch (currentState) {
    case OFF: // Nothing happening, waiting for switchInput
      analogWrite(enA, 0);
      analogWrite(enB, 0);
      mySerial.println("OFF");
      if (cmd == "on" || buttonCommand) {
        buttonCommand = false;
        currentState = MOVE;
        break;
      }
      else {        
        currentState = OFF;
        break;
      }

    case MOVE:
      mySerial.println(z);
      updateZ();
      if (cmd == "off" || buttonCommand) {
        buttonCommand = false;
        currentState = OFF;
        timer = millis();
        break;
      }       
      mySerial.println("Moving!!!!!");   
      analogWrite(enA, 200);
      analogWrite(enB, 200); 
                                                         
      if (z > z_init+5 && z < 45) {
        updateZ();
        analogWrite(enA, 160);
        analogWrite(enB, 190);
      }
      else if (z > z_init+5 && z > 45){
        updateZ();
        analogWrite(enA, 120);
        analogWrite(enB, 170);        
      }
      if (z < z_init-5 && z > -45) {
        updateZ();
        analogWrite(enA, 190);
        analogWrite(enB, 160);
      }
      else if (z < z_init-5 && z < -45){
        updateZ();
        analogWrite(enA, 170);
        analogWrite(enB, 120);        
      }
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
}

void updateZ(){
  z = -(mpu.getAngleZ());
}

void obstacleCheck() {
  // clear trigpin
  digitalWrite(tPin, LOW);
  delayMicroseconds(2);
  // set trigpin to high for 15 microseconds
  digitalWrite(tPin, HIGH);
  delayMicroseconds(15);
  digitalWrite(tPin, LOW);
  // reads echo pin and returns microseconds of the sound wave travel time
  duration = pulseIn(ePin, HIGH);
  // Calculating the distance
  dist = duration * 0.034 / 2;
  while (dist < 15) {
    analogWrite(enA, 0);
    analogWrite(enB, 0);
  }
}

#include <SoftwareSerial.h>
#include <MPU6050_light.h>
#include "Wire.h"


//communication pins
#define rxPin 8
#define txPin 9

//push button pin
#define buttonPin 12

//ultrasonic sensor pin
#define usPin 2

//IR pin
#define irPin A0

//left motor pins
#define enA 11
#define in1 7
#define in2 6
//right motor pins
#define enB 10
#define in3 5
#define in4 4

#define SpeedL 100-9
#define SpeedR 97-10

// Set up a new SoftwareSerial object for bluetooth
SoftwareSerial mySerial(8,9);


MPU6050 mpu(Wire);

unsigned long timer = 0;

// states the tractor could be in
enum {OFF, MOVE, TURN_L, TURN_R, TURN180};
unsigned char currentState;  // tractor state at any given moment

// states the button can be in
enum {PUSHED, RELEASED};

unsigned char buttonState; // button state at any given moment
bool buttonCommand; // boolean conversion from button input
int buttonRead; // command boolean used for directing tractor FSM logic
int usRead;
int irRead;
int numTapes = 0;
unsigned long debounceDelay = 50;
unsigned long debounceTime = 0;

float elapsedTime = millis()/1000;

// bluetooth char
String cmd;
// gyroscope float
float z;
float z_init;

void setup() {
  // Begins serial communication

  Serial.begin(9600);
  // Defines button pin as input
  pinMode(buttonPin, INPUT);

  // Define ultrasonic sensor pin as input
  pinMode(usPin, INPUT);
  pinMode(irPin, INPUT);
  pinMode(rxPin,INPUT);
  pinMode(txPin,OUTPUT);
  //left motor
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  //right motor
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  //init direction for left motor
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  //init direction for right motor
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  //Serial setup
  mySerial.begin(9600);
  
  //gyro setup
  gyroSetup();

  //init state
  currentState = OFF;
  buttonState = RELEASED;
  buttonCommand = false;

}

void loop() {
   /*if(millis() - timer > 2000){ // print data every 2 seconds to bluetooth module
    mySerial.print("\tZ: ");mySerial.println(mpu.getGyroZ());
    mySerial.print("\tZ: ");mySerial.println(mpu.getAngleZ());
    mySerial.println(F("===================="));
    timer = millis();
  }*/
  elapsedTime = millis()/1000;
  updateZ();
  //mySerial.print(static_cast<int>(mpu.getAccY()));
  if (mySerial.available()) {
    cmd = mySerial.readString();
    Serial.print("Command: ");

    
    if (cmd == "off") {
      Serial.println("Turning off Robot");
      mySerial.print(numTapes);
      //mySerial.print(distTraveled);
    }
    else if (cmd == "on") {
      Serial.println("Turning on Robot");
    }
    else if (cmd == "left") {
      Serial.println("Turning left");
    }
    else if (cmd == "right") {
      Serial.println("Turning right");
    }
    else {
      Serial.println("Invalid command");
    }
    
  }
  updateZ();
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
      usRead = digitalRead(usPin);
      Serial.println("Off");
      analogWrite(enA, 0);
      analogWrite(enB, 0);
      updateZ();
      if (cmd == "on" || buttonCommand) {
        buttonCommand = false;
        timer = millis();
        currentState = MOVE;
        break;
      }
      else if (cmd == "left"){
        currentState = TURN_L;
        updateZ();
        break;
      }
      else if (cmd == "right"){
        currentState = TURN_R;
        updateZ();
        break;
      }
      break;

    case MOVE:
      updateZ();
      // if ((millis()-timer)>3000){
      //   currentState = OFF;
      //   mpu.update();
      //   break;
      // }
      Serial.println("On");
      if (cmd == "off" || buttonCommand) {
        buttonCommand = false;
        currentState = OFF;
        mySerial.print(numTapes);
        timer = millis();
        updateZ();
        break;
      }
      else if (cmd == "left"){
        currentState = TURN_L;
        updateZ();
        break;
      }
      else if (cmd == "right"){
        currentState = TURN_R;
        updateZ();
        break;
      }
      usRead = digitalRead(usPin);
      irRead = analogRead(irPin);
      Serial.println(irRead);
      if ((irRead < 700)){
        numTapes++;
        if (numTapes<=2){
          //turn left
          currentState = TURN_L;
          mySerial.println("LEFT");
          break;
        }
        else if (numTapes>=3 && numTapes<=4){
          //turn right
          currentState = TURN_R;
          mySerial.println("RIGHT");
          break;
        }
        else{
          //turn left 
          currentState = TURN_L;     
          mySerial.println("LEFT"); 
          break;   
        }
        mpu.update();
        break;
      }
      if ((usRead == 1)){
        currentState = OFF;
        updateZ();
        break;
      }      
      updateZ();

      if (z > z_init+5) {
        Serial.println("Correcting to right");
        updateZ();
        Serial.println(z);
        analogWrite(enA, SpeedL);//110
        analogWrite(enB, 0);//140
        break;
      }
      if (z < z_init-5) {
        Serial.println("Correcting to left");
        updateZ();
        Serial.println(z);
        analogWrite(enA, 0);
        analogWrite(enB, SpeedR);
        break;
      }
      analogWrite(enA, SpeedL);
      analogWrite(enB, SpeedR);                
      updateZ();
      break;      

    case TURN_R:  
      updateZ();
      while (z > z_init-90 )
      {
        analogWrite(enA, SpeedL - 35);
        analogWrite(enB, 0);
        updateZ();
      } 
      reinitialize();
      currentState = MOVE;
      // Resets the motor direction to initial values and establishes a new inital Z
      // Switches to MOVE state so robot continues along path
      break;

      
    case TURN_L: 
      updateZ();
      while (z < z_init+90 )
      {
        analogWrite(enA, 0);
        analogWrite(enB, SpeedR - 35);
        updateZ();
      } 
      reinitialize();
      currentState = MOVE;
      // Resets the motor direction to initial values and establishes a new inital Z
      // Switches to MOVE state so robot continues along path
      break;
    case TURN180:
      break;
    
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
  mpu.update();
  z = (mpu.getAngleZ());
}

void gyroSetup(){
  Wire.begin();
  mpu.begin();
  Serial.println(("Calculating offsets, do not move robot"));
  delay(1000);
  mpu.calcOffsets(true,true); // gyro and accelero
  mpu.setFilterGyroCoef(0.98);
  Serial.println("Done!\n");
  z_init = mpu.getAngleZ();
}

void reinitialize() {  
  //gyro reset
  z_init = mpu.getAngleZ();
}



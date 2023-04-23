// LIBRARIES

#include <SoftwareSerial.h>
#include <MPU6050_light.h>
#include "Wire.h"

/*========================================================================================*/

// MACROS

//bluetooth module communication pins
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

//default speeds for PWM to left and right wheel (derived through testing)
#define SpeedL 100-9
#define SpeedR 97-10

/*========================================================================================*/

// OBJECT CONSTRUCTORS

// Creates an instance of a SoftwareSerial object
SoftwareSerial mySerial(rxPin,txPin);

// Constructor for gyroscope
MPU6050 mpu(Wire);

/*========================================================================================*/

// TRACTOR FSM

// states the tractor could be in
enum {OFF, MOVE, TURN_L, TURN_R, TURN180};
unsigned char currentState;  // tractor state at any given moment

/*========================================================================================*/

// BUTTON FSM and VARIABLES

// states the button can be in
enum {PUSHED, RELEASED};

unsigned char buttonState; // button state at any given moment
bool buttonCommand = false; // boolean conversion from button input
int buttonRead; // command boolean used for directing tractor FSM logic
unsigned long debounceDelay = 50;
unsigned long debounceTime = 0;
unsigned long timer = 0;

/*========================================================================================*/

// PERIPHERALS

int usRead; // integer to store ultrasonic sensor digital read
int irRead; // integer to store IR sensor analog read
int numTapes = 0; // number of black tapes encountered

/*========================================================================================*/

// OTHER INFORMATION

// Elapsed time in seconds
float elapsedTime = millis()/1000;

// String to store received command from Bluetooth
String cmd;

// Variables to store relevant gyroscopic information (degree rotation about Z)
float z;
float z_init;

/*========================================================================================*/

// Initial Setup

void setup() {
  // Begins serial communication
  Serial.begin(9600);
  
  // Defines button pin as input
  pinMode(buttonPin, INPUT);

  // Defines ultrasonic sensor pin as input
  pinMode(usPin, INPUT);
  pinMode(irPin, INPUT);
  
  // Defines RX pin of bluetooth module as input (send data)
  pinMode(rxPin,INPUT);

  // Defines TX pin of bluetooth module as output (receive data)
  pinMode(txPin,OUTPUT);
  
  // Defines speed and direction pins for left motor
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  // Defines speed and direction pins for right motor
  pinMode(enB, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Initial direction for left motor
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  
  // Initial direction for right motor
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  // Establishes serial communication between bluetooth module and arduino
  mySerial.begin(9600);
  
  // Calls gyroSetup() helper function to initialize gyroscope
  gyroSetup();

  // Initial states of FSM's
  currentState = OFF;
  buttonState = RELEASED;

}

void loop() {
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

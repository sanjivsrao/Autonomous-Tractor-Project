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
  elapsedTime = millis()/1000; // Updates elapsed time each loop
  updateZ();
  read_command(); // Checks if command are available on bluetooth module, if so then reads command and stores in cmd
  updateZ();

  
  // Button switch debounce FSM
  switch (buttonState) {
    case PUSHED:
      buttonRead = digitalRead(buttonPin);
      if (!buttonRead){
        buttonState=RELEASED;
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


  // Main tractor state FSM
  switch (currentState) {
    case OFF:
      usRead = digitalRead(usPin);
      Serial.println("Off");
      analogWrite(enA, 0);
      analogWrite(enB, 0);
      updateZ();
      // If the command is "on" or the button is pressed, the state switches to MOVE
      if (cmd == "on" || buttonCommand) {
        buttonCommand = false;
        timer = millis();
        currentState = MOVE;
        break;
      }
      // If the command is "left", the state switches to TURN_L
      else if (cmd == "left"){
        currentState = TURN_L;
        updateZ();
        break;
      }
      // If the command is "right", the state switches to TURN_R
      else if (cmd == "right"){
        currentState = TURN_R;
        updateZ();
        break;
      }
      break;

    case MOVE:
      updateZ();
      Serial.println("On");
      // If the command is "off" or the button is pressed, the state switches to OFF
      if (cmd == "off" || buttonCommand) {
        buttonCommand = false;
        currentState = OFF;
        mySerial.print(numTapes);
        timer = millis();
        updateZ();
        break;
      }
      // If the command is "left", the state switches to TURN_L
      else if (cmd == "left"){
        currentState = TURN_L;
        updateZ();
        break;
      }
      // If the command is "right", the state switches to TURN_R
      else if (cmd == "right"){
        currentState = TURN_R;
        updateZ();
        break;
      }




      /* Black line detection:
       * This section of code reads the value on the irPin and checks if it
       * is below a certain threshold for the black tape. If the value is
       * below the threshold, the robot registers the black tape and switches
       * to the appropriate state for the course provided. Based on the number
       * of tapes, the robot will turn right or left.
      */
//      irRead = analogRead(irPin);
//      // Detects the reflectivity of surface
//      if ((irRead < 700)){
//        numTapes++;
//        if (numTapes<=2){
//          //turn left
//          currentState = TURN_L;
//          mySerial.println("LEFT");
//          break;
//        }
//        else if (numTapes>=3 && numTapes<=4){
//          //turn right
//          currentState = TURN_R;
//          mySerial.println("RIGHT");
//          break;
//        }
//        else{
//          //turn left 
//          currentState = TURN_L;     
//          mySerial.println("LEFT"); 
//          break;   
//        }
//        mpu.update();
//        break;
//      }


      /* Object detection:
       * If the value is 1, an the ultrasonic sensor is detecting an object.
       * The robot then switches to the OFF state and stops.
      */
      // Reads the value on the ultrasonic sensor
      usRead = digitalRead(usPin);

      if ((usRead == 1)){
        currentState = OFF;
        updateZ();
        break;
      }      


      /* Pathfinding algorithm:
       * If there is a 5 degree difference between the intial Z value and
       * the current Z value, the robot will correct towards the Z_initial
      */
      updateZ();
      if (z > z_init+5) {
        Serial.println("Correcting to right");
        updateZ();
        Serial.println(z);
        analogWrite(enA, SpeedL);
        analogWrite(enB, 0);
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

      // Default move speeds
      analogWrite(enA, SpeedL);
      analogWrite(enB, SpeedR);                
      updateZ();
      break;      

    case TURN_R:  
      updateZ();
      // Blocking code that sets a target change in degrees then turns the robot till that value is reached
      while (z > z_init-90 )
      {
        analogWrite(enA, SpeedL - 35);
        analogWrite(enB, 0);
        updateZ();
      } 
      reinitialize();
      currentState = MOVE;
      break;

      
    case TURN_L: 
      updateZ();
      // Blocking code that sets a target change in degrees then turns the robot till that value is reached
      while (z < z_init+90 )
      {
        analogWrite(enA, 0);
        analogWrite(enB, SpeedR - 35);
        updateZ();
      } 
      reinitialize();
      currentState = MOVE;
      break;
      
  }
  // Clears the command
  cmd = "";
}


// Simple function to establish a new value for Z initial
void reinitialize() {  
  z_init = mpu.getAngleZ();
}




/*========================================================================================
Function: This function simple updates the gyroscope, using mpu.update(), to get updated
values for all gyroscope angles, then assigns the current angle of the Z axis to var z.

Args: None

Return: None

Notes: This function is called often to constantly update the angle of rotation of the
gyroscope. The more often the function is called, the more accurately the robot can
adjust itself to stay on course. 
========================================================================================*/
void updateZ(){
  mpu.update();
  z = (mpu.getAngleZ());
}





/*========================================================================================
Function: This function calls all the components of the gyroscope setup based on the
MPU6050_light.h library. A simple delay is included to allow the gyroscope enough time to
properly calibrate. If connected to the Arduino IDE, the function will display "Done!" in
the serial monitor once calibration is complete. It also will establish the initial value
of z. 

Args: None

Return: None

Notes: This function is called only once in setup. It was put into a function for code
cleanliness and ease of debugging. 
========================================================================================*/
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




/*========================================================================================
Function: This function checks if a command is available in the bluetooth module. If so,
it reads the string and prints it to the serial monitor. Alongside this, it filters the
command to see if it is one of the predefined robot commands. If the command is the off
command, it will send the number of black tapes encountered to the bluetooth module.

Args: None

Return: None

Notes: This is how the bluetooth module sends the number of black tapes to the GUI.
========================================================================================*/
void read_command() {
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
}

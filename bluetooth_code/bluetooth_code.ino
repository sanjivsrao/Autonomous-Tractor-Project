#include <SoftwareSerial.h>

//bluetooth module communication pins
#define rxPin 8
#define txPin 9

// Set up a new SoftwareSerial object for bluetooth
SoftwareSerial mySerial(rxPin,txPin);

void setup()
{
mySerial.begin(9600);   
Serial.begin(9600);   
delay(100);
}
void loop()
{
String cmd;
while (mySerial.available()==0){}
cmd = mySerial.readString();
if (cmd == "off"){
  mySerial.println("Turning off Robot");
}
else if (cmd == "on"){
  mySerial.println("Turning on Robot");
}
else{
  mySerial.println("Invalid command");
}
}

String cmd_read(){
  String cmd;
  while (mySerial.available()==0){}
  cmd = mySerial.readString();
  if (cmd == "off"){
    mySerial.println("Turning off Robot");
  }
  else{
    mySerial.println("Invalid command");
  }
}

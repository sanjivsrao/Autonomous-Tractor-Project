#include <Wire.h>
#include <MPU6050_light.h>

MPU6050 mpu(Wire);
unsigned long timer = 0;
 
void setup() {
  Serial.begin(9600);                                     
  Wire.begin();
  mpu.begin();     
  mpu.calcGyroOffsets();      
}
 
void loop() {
  mpu.update();
  float x = mpu.getAngleX();
  float y = mpu.getAngleY();
  float z = -(mpu.getAngleZ());
  if((millis()-timer)>1000) 
  {                                                                
    Serial.print("Roll: ");
    Serial.println(x);
    Serial.print("Pitch: ");
    Serial.println(y);
    Serial.print("Yaw: ");
    Serial.print(z);
    Serial.println("\n");
    timer = millis();  
  }
}

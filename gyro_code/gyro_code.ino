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
    
  if((millis()-timer)>1000) 
  {                                                                
    Serial.print("Pitch: ");
    Serial.println(mpu.getAngleX());
    Serial.print("Roll: ");
    Serial.println(mpu.getAngleY());
    Serial.print("Yaw: ");
    Serial.print(mpu.getAngleZ());
    Serial.println("\n");
    timer = millis();  
  }
}
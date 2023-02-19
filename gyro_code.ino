#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

void setup(void) {
	Serial.begin(115200);

	// Initialization code which prints an error message if chip was not located
	if (!mpu.begin()) {
		Serial.println("Failed to find MPU6050 chip");
		while (1) {
		  delay(10);
		}
	}
  // set accelerometer range to +- 2g (using smallest acceleration range for extra precision)
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
	// set gyro range to +- 1000 deg/s (using a less precise measurement range)
	mpu.setGyroRange(MPU6050_RANGE_500_DEG);

	// set filter bandwidth to 44 Hz (arbitrary value)
	mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);

	delay(100);
}

void loop() {
	/* Get new sensor events with the readings */
	sensors_event_t g, a;
	mpu.getEvent(&a, &g);

	/* Print out the values */
	Serial.print(a.acceleration.x);
	Serial.print(",");
	Serial.print(a.acceleration.y);
	Serial.print(",");
	Serial.print(a.acceleration.z);

	Serial.println("");

  Serial.print("Rotation X: ");
	Serial.print(g.gyro.x);
	Serial.print(", Y: ");
	Serial.print(g.gyro.y);
	Serial.print(", Z: ");
	Serial.print(g.gyro.z);
	Serial.println(" rad/s");
}
/*
This example shows how to set up and read multiple VL53L1X sensors connected to
the same I2C bus. Each sensor needs to have its XSHUT pin connected to a
different Arduino pin, and you should change sensorCount and the xshutPins array
below to match your setup.

For more information, see ST's application note AN4846 ("Using multiple VL53L0X
in a single design"). The principles described there apply to the VL53L1X as
well.
*/

#include <Wire.h>
#include <VL53L1X.h>

// The number of sensors in your system.
const uint8_t sensorCount = 4;
//const uint8_t sensorCount = 1;

// The Arduino pin connected to the XSHUT pin of each sensor.
const uint8_t xshutPins[sensorCount] = { 2,3, 8,9 };
//const uint8_t xshutPins[sensorCount] = { 9 };

VL53L1X sensors[sensorCount];

unsigned long time = 0;

void setup() {

  time = millis();
  while (!Serial) {}
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C

  // Disable/reset all sensors by driving their XSHUT pins low.
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }

  // Enable, initialize, and start each sensor, one by one.
  for (uint8_t i = 0; i < sensorCount; i++)
  {
    // Stop driving this sensor's XSHUT low. This should allow the carrier
    // board to pull it high. (We do NOT want to drive XSHUT high since it is
    // not level shifted.) Then wait a bit for the sensor to start up.
    pinMode(xshutPins[i], INPUT);
    delay(10);

    sensors[i].setTimeout(500);
    if (!sensors[i].init())
    {
      Serial.print("Failed to detect and initialize sensor ");
      Serial.println(i);
      while (1);
    }

    // Each sensor must have its address changed to a unique value other than
    // the default of 0x29 (except for the last one, which could be left at
    // the default). To make it simple, we'll just count up from 0x2A.
    sensors[i].setAddress(0x2A + i);

    sensors[i].startContinuous(50);
    sensors[i].setDistanceMode(VL53L1X::Long);
    //sensors[i].setDistanceMode(VL53L1X::Medium);
    //sensors[i].setDistanceMode(VL53L1X::Short);
  }
	delay(2000);
	Serial.println("Arduino OK");
}

void loop() {
	if (millis() > time + 1000) {
		time = millis();
		Serial.print("[");
		for (uint8_t i = 0; i < sensorCount; i++) {
			if (i > 0) {
				Serial.print(",");
			}
			Serial.print(sensors[i].read());
			if (sensors[i].timeoutOccurred()) {
				Serial.print(",0,");
			}
			else {
				Serial.print(",1,");
			}
			Serial.print((int)sensors[i].ranging_data.peak_signal_count_rate_MCPS);
	 	}
		Serial.println("]");
	}
}

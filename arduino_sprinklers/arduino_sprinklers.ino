#include "Arduino.h"
//define pins for hydro valves relays
const int RELAYS_PINS[] = { 3, 4, 5, 6 };
const int WELL_PUMP_PIN = 7;
const int TANK_FULL_PIN = 8;
int LED = 13; // Use the onboard Uno LED

void setup() {
	Serial.begin(9600);
	pinMode(TANK_FULL_PIN, INPUT);

// Add your initialization code here
}

// The loop function is called in an endless loop
void loop() {
	//read tank moisture sensor
	int tankFull = digitalRead(TANK_FULL_PIN);
	Serial.println("Tank full: " + tankFull);

	delay(5);
}

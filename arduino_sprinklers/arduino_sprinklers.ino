//#define BLYNK_DEBUG
//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <SimpleTimer.h>

// Set ESP8266 Serial object
#define EspSerial Serial
#define PIN_WATER_SENSOR 0
#define PIN_WELL_PUMP 11
#define VPIN_HYDRO 5
#define PIN_HYDRO 10
#define VPIN_SLIDER_TIME 0
#define DEBUG

int sensor_water = A0;
WidgetLCD lcd(V3);

ESP8266 wifi(&EspSerial);
//boolean debug = false;
#ifdef DEBUG
//debug auth
char auth[] = "70be1068f16d4d8097f884ce898c98ea";
#else
//release auth
char auth[] = "77a8fc1fbd2046299004d28217bc4199";
#endif

const char* SSID = "babilons";
const char* PASS = "thai0lai5";

//check if tank is full(seconds)
short checkTankFullInterval = 2;

SimpleTimer timer;

int value_water;
//default watering time
int wateringTimeSliderValueDefault = 70;
int wateringTimeSliderValue;
int resetSliderValue;
boolean tankFull = false;
short hydroState = 0;
short wateringTime = 0;
boolean tankFullAtStart = false;
short autoStopFailSafe = 3;

BLYNK_WRITE(VPIN_SLIDER_TIME)
{
	//   BLYNK_LOG("Got a value: %s", param.asStr());
	wateringTimeSliderValue = param.asInt();
	//  Blynk.virtualWrite(V1, wateringTimeSliderValue); //show slider value on vDisplay with Push setting
}

BLYNK_WRITE(VPIN_HYDRO)
{
	if (param.asInt() == 1) {
		digitalWrite(PIN_HYDRO, 0);
		hydroState = 1;
		wateringTime = 0;
		if (tankFull) {
			tankFullAtStart = true;
		} else {
			tankFullAtStart = false;
		}
	} else {
		digitalWrite(PIN_HYDRO, 1);
		hydroState = 0;
	}
}

void fillTankWhenWatering() {
	if (hydroState == 1) {
		short waterLevel = analogRead(PIN_WATER_SENSOR);
		//tank not full
		if (waterLevel > 800) {
			digitalWrite(PIN_WELL_PUMP, LOW);
			tankFull = false;
			printToLcd(1, "Filling tank...");
		}
	}
}

void stopWatering() {
	++wateringTime;
	if (hydroState == 1) {
		String tmp = "Time: ";
		tmp += wateringTime;
		printToLcd(0, tmp);
	}

	short stopTime =
			tankFullAtStart ? wateringTimeSliderValue : autoStopFailSafe;

	if (wateringTime >= stopTime && hydroState == 1) {
		//stop hydro
		digitalWrite(PIN_HYDRO, HIGH);
		hydroState = 0;
		Blynk.virtualWrite(VPIN_HYDRO, 0);
		String tmp1 = "FINISH,Time ";
		tmp1 += wateringTime;
		printToLcd(0, tmp1);
		Blynk.notify("Watering finished,snails say HI!");
	}
}

void checkTankFull() {
	//tank is full
	if (tankFull == false) {
		if (analogRead(PIN_WATER_SENSOR) < 800) {
			printToLcd(1, "Tank full");
			digitalWrite(PIN_WELL_PUMP, HIGH);
			tankFull = true;
		}
	}
}

void checkWaterTank() {
	if (!tankFull)
		return;
	value_water = analogRead(PIN_WATER_SENSOR);
	//tank status is full but now sensor reports its not full
	if (value_water > 800) {
		tankFull = false;
		printToLcd(1, "Filling tank...");
		digitalWrite(PIN_WELL_PUMP, LOW);
	}
}

void checkTankFullColdStart() {
	tankFull = false;
	//tank is not full
	checkTankFull();
	if (!tankFull) {
		digitalWrite(PIN_WELL_PUMP, LOW);
		printToLcd(1, "Filling tank...");
	} else {
		printToLcd(1, "Tank full");
	}
}

void printToLcd(short line, String message) {
	lcd.print(0, line, "                ");
	lcd.print(0, line, message);
}

void setup() {
	unsigned long second = 1000L;
	unsigned long minute = 60 * second;
	unsigned long hour = 60 * minute;

	// Set console baud rate
	Serial.begin(9600);
	delay(10);
	// Set ESP8266 baud rate
	EspSerial.begin(115200);
	//when after blackout, wait for router to come back
#ifdef DEBUG
	delay(10);
#else
	delay(80000L);
#endif

	Blynk.begin(auth, wifi, SSID, PASS);

	pinMode(PIN_WATER_SENSOR, INPUT);
	pinMode(PIN_WELL_PUMP, OUTPUT);
	pinMode(PIN_HYDRO, OUTPUT);

	//set well and sprinklers pump off by default
	digitalWrite(PIN_WELL_PUMP, HIGH);
	digitalWrite(PIN_HYDRO, HIGH);

	unsigned long checkTankInterval = 20 * minute;
	unsigned long wateringRoutineInterval = 1 * minute;
	unsigned long stopWateringInterval = minute;

#ifdef DEBUG
	stopWateringInterval = second;
	checkTankInterval = second * 5;
#endif

	timer.setInterval(checkTankInterval, checkWaterTank);
	timer.setInterval(second * 35, fillTankWhenWatering);
	timer.setInterval(stopWateringInterval, stopWatering);
	//check if tank is full
	timer.setInterval(second * checkTankFullInterval, checkTankFull);

	while (Blynk.connect() == false) {
		// Wait until connected
	}
	Blynk.virtualWrite(VPIN_HYDRO, 0);
	Blynk.virtualWrite(VPIN_SLIDER_TIME, wateringTimeSliderValueDefault);
	wateringTimeSliderValue = wateringTimeSliderValueDefault;

	//around 40 chars can be seen on phone
	Blynk.notify("Snails are back online and thirsty ;)");
	printToLcd(0, "READY");
	checkTankFullColdStart();
}

void loop() {
	Blynk.run();
	timer.run();
}

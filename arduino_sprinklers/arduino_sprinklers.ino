//#define BLYNK_DEBUG
//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
//#include <Blynk.h>
//#include <BlynkESP8266_Lib.h>
//#include <ESP8266_Lib.h>
//#include <BlynkSimpleShieldEsp8266.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include "DHT.h"

//OTA
//#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
//#include <ESP8266HTTPUpdateServer.h>

//OTA new
#include <ArduinoOTA.h>

// Set ESP8266 Serial object
//#define EspSerial Serial
#define PIN_WATER_SENSOR 4
#define PIN_WELL_PUMP 5
#define PIN_HYDRO 12
//dht22
#define DHTPIN 13
int sensor_water = A0;

#define VPIN_HYDRO 5
#define VPIN_SLIDER_TIME 0
#define VPIN_TEMP 1
#define VPIN_HUM 2

#define DEBUG

//dht22
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

//OTA
//ESP8266WebServer httpServer(80);
//ESP8266HTTPUpdateServer httpUpdater;

WidgetLCD lcd(V3);

//ESP8266 wifi(&EspSerial);
//boolean debug = false;
#ifdef DEBUG
//debug auth
char auth[] = "70be1068f16d4d8097f884ce898c98ea";
#else
//release auth
char auth[] = "77a8fc1fbd2046299004d28217bc4199";
#endif

const char* host = "esp8266-webupdate";
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

//telnet serial
//const uint16_t aport = 23;
//WiFiServer TelnetServer(aport);
//WiFiClient TelnetClient;

BLYNK_WRITE(VPIN_SLIDER_TIME) {
	//   BLYNK_LOG("Got a value: %s", param.asStr());
	wateringTimeSliderValue = param.asInt();
	//  Blynk.virtualWrite(V1, wateringTimeSliderValue); //show slider value on vDisplay with Push setting
}

BLYNK_WRITE(VPIN_HYDRO) {
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

void checkTempAndHum() {
	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	float h = dht.readHumidity();
	// Read temperature as Celsius (the default)
	float t = dht.readTemperature();
	// Read temperature as Fahrenheit (isFahrenheit = true)
	float f = dht.readTemperature(true);

	// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t) || isnan(f)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

	// Compute heat index in Fahrenheit (the default)
//  hif = dht.computeHeatIndex(f, h);
	// Compute heat index in Celsius (isFahreheit = false)
	t = dht.computeHeatIndex(t, h, false);

	Blynk.virtualWrite(VPIN_TEMP, t);
	Blynk.virtualWrite(VPIN_HUM, h);
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
//	EspSerial.begin(9600);
	//when after blackout, wait for router to come back
#ifdef DEBUG
	delay(10);
#else
	delay(80000L);
#endif

	//OTA
//	WiFi.mode(WIFI_AP_STA);

	//OTA new
	WiFi.mode(WIFI_STA);

	Blynk.begin(auth, SSID, PASS);

	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		WiFi.begin(SSID, PASS);
		Serial.println("WiFi failed, retrying.");
	}

	while (Blynk.connect() == false) {
		// Wait until connected
	}

	//OTA
//	MDNS.begin(host);
//
//	httpUpdater.setup(&httpServer);
//	httpServer.begin();
//
//	MDNS.addService("http", "tcp", 80);
//
//	delay(10 * 1000);
//	Serial.printf(
//			"HTTPUpdateServer ready! Open http://%s.local/update in your browser\n",
//			host);
//	Serial.println("********************************************");
//	Serial.printf("http://");
//	Serial.print(WiFi.localIP());
//	Serial.print("/update sssss\n");

	//OTA new
	ArduinoOTA.begin();

//dht22
	dht.begin();

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
	//dht22
	timer.setInterval(2000, checkTempAndHum);

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
	//OTA
//	httpServer.handleClient();
	//OTA new
	ArduinoOTA.handle();
}

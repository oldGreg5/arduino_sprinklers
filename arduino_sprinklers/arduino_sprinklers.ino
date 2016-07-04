//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <SimpleTimer.h>

// Set ESP8266 Serial object
#define EspSerial Serial
#define PIN_WATER_SENSOR 0
#define PIN_WELL_PUMP 11
#define VPIN_HYDRO 5
#define PIN_HYDRO 10
#define VPIN_SLIDER_RESET 2
#define VPIN_SLIDER_TIME 0
//#define VPIN_START 1
//#define VPIN_S1 6
//#define VPIN_S2 7
//#define VPIN_S3 8
//#define VPIN_S4 9
//#define PIN_S1 4
//#define PIN_S2 5
//#define PIN_S3 6
//#define PIN_S4 7
int sensor_water = A0;
WidgetLCD lcd(V3);

ESP8266 wifi(&EspSerial);
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
boolean debug = false;
//release auth
char auth[] = "77a8fc1fbd2046299004d28217bc4199";
//debug auth
//char auth[] = "70be1068f16d4d8097f884ce898c98ea";

const char* SSID = "babilons";
const char* PASS = "thai0lai5";

//check if tank is full(seconds)
short checkTankFullInterval = 2;

SimpleTimer timer;

int value_water;
//default watering time
int wateringTimeSliderValueDefault = 25;
int wateringTimeSliderValue;
int resetSliderValue;
boolean tankFull = false;
//0 cold start,1 START ,2 PAUSE
//short routineState = 0;
//short startButton = 0;
//short sprinklers = 4;
//int sprinklersStatus[4];
//int sprinklers[4];
//int finishedSprinklers = 0;
short hydroState = 0;
short wateringTime = 0;
boolean tankFullAtStart = false;
short autoStopWateringAfter = 25;

BLYNK_WRITE(VPIN_SLIDER_TIME)
{
	//   BLYNK_LOG("Got a value: %s", param.asStr());
	wateringTimeSliderValue = param.asInt();
	//analogWrite(1,wateringTimeSliderValue);
	//  Blynk.virtualWrite(V1, wateringTimeSliderValue); //show slider value on vDisplay with Push setting
}

//BLYNK_WRITE(VPIN_START)
//{
//	//   BLYNK_LOG("Got a value: %s", param.asStr());
//	startButton = param.asInt();
//	//  wateringTimeSliderValue = wateringTimeSliderValueDefault;
//	if (startButton == 1) {
//		//start routine
//		//if (routineState == 0 || routineState == 2) {
//		routineState = 1;
//		digitalWrite(PIN_HYDRO , 0);
//		wateringRoutine();
//		//}
//	} else {
//		routineState = 2;
//		digitalWrite(PIN_HYDRO , 1);
//		for (int j = 0; j < 4; j++) {
//			digitalWrite(sprinklers[j] , 1);
//		}
//	}
//	//analogWrite(1,wateringTimeSliderValue);
//	//  Blynk.virtualWrite(V1, startButton); //show slider value on vDisplay with Push setting
//}

BLYNK_WRITE(VPIN_SLIDER_RESET)
{
	//when set to 100 reset routine to initial values
	resetSliderValue = param.asInt();
	//analogWrite(1,wateringTimeSliderValue);
	if (resetSliderValue == 100) {
		resetWateringRoutine();
	}
	//  Blynk.virtualWrite(V1, resetSliderValue); //show slider value on vDisplay with Push setting
}

BLYNK_WRITE(VPIN_HYDRO)
{
	//  resetSliderValue = param.asInt();
	//analogWrite(1,wateringTimeSliderValue);
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
	//  Blynk.virtualWrite(V1, resetSliderValue); //show slider value on vDisplay with Push setting
}

//BLYNK_WRITE(VPIN_S1)
//{
//	if (param.asInt() == 1) {
//		digitalWrite(PIN_S1, 0);
//	} else {
//		digitalWrite(PIN_S1, 1);
//	}
//}
//
//BLYNK_WRITE(VPIN_S2)
//{
//	if (param.asInt() == 1) {
//		digitalWrite(PIN_S2, 0);
//	} else {
//		digitalWrite(PIN_S2, 1);
//	}
//}
//
//BLYNK_WRITE(VPIN_S3)
//{
//	if (param.asInt() == 1) {
//		digitalWrite(PIN_S3, 0);
//	} else {
//		digitalWrite(PIN_S3, 1);
//	}
//}
//
//BLYNK_WRITE(VPIN_S4)
//{
//	if (param.asInt() == 1) {
//		digitalWrite(PIN_S4, 0);
//	} else {
//		digitalWrite(PIN_S4, 1);
//	}
//}

//BLYNK_WRITE(VPIN_SLIDER_RESET)
//{
//  int reset = param.asInt();
//  //analogWrite(1,wateringTimeSliderValue);
//  if (reset == 1) {
//    Blynk.virtualWrite(VPIN_SLIDER_RESET,0);
//  }
//  //  Blynk.virtualWrite(V1, resetSliderValue); //show slider value on vDisplay with Push setting
//}

void checkWaterTank() {
	//  if (!tankFull) {
	value_water = analogRead(PIN_WATER_SENSOR);
	//tank status is full but now sensor reports its not full
	if (value_water > 800 && tankFull == true) {
		//    tankFull = false;
		//    lcd.print(0, 1, "Tank full");
		//    digitalWrite (PIN_WELL_PUMP, LOW);
		//    timerWell.restartTimer(1);
		//  } else {
		tankFull = false;
		printToLcd(1, "Filling tank...");
//		lcd.print(0, 1, "                ");
//		lcd.print(0, 1, "Filling tank...");
		digitalWrite(PIN_WELL_PUMP, LOW);
		//  } else {
		//    lcd.print(0, 1, "Tank full");
		//    digitalWrite (PIN_WELL_PUMP, HIGH);
		//  }
	}
}

void resetWateringRoutine() {
	//turn off all valves
//	Blynk.virtualWrite(VPIN_S1, 0);
//	Blynk.virtualWrite(VPIN_S2, 0);
//	Blynk.virtualWrite(VPIN_S3, 0);
//	Blynk.virtualWrite(VPIN_S4, 0);
//	Blynk.virtualWrite(VPIN_START, 0);
	Blynk.virtualWrite(VPIN_HYDRO, 0);
	Blynk.virtualWrite(VPIN_SLIDER_TIME, wateringTimeSliderValueDefault);
	wateringTimeSliderValue = wateringTimeSliderValueDefault;
	Blynk.virtualWrite(VPIN_SLIDER_RESET, 0);

	//set default sprinklers time to 0 minutes
//	for (int i = 0; i < 4; i++) {
//		sprinklersStatus[i] = 0;
//		digitalWrite(sprinklers[i], 1);
//	}

//	digitalWrite(PIN_HYDRO, HIGH);
//
//	if (routineState == 2) {
//		printToLcd(0, "Status: FINISHED");
////		lcd.print(0, 0, "                ");
////		lcd.print(0, 0, "Status: FINISHED");
//	} else {
//		routineState = 0;
	//  lcd.clear(); //Use it to clear the LCD Widget
	printToLcd(0, "Status: RESET");
//		lcd.print(0, 0, "                ");
//		lcd.print(0, 0, "Status: RESET");
//	}
}

//void wateringRoutine() {
//	//start routine
//	if (routineState == 1) {
//		for (int i = 0; i < 4; i++) {
//
//			//      lcd.print(0, 1, "                ");
//			//      String tmp1 = "*";
//			//      tmp1 += sprinklersStatus[i];
//			//      tmp1 += "/";
//			//      tmp1 += wateringTimeSliderValue;
//			//      lcd.print(0, 1, tmp1);
//			if (sprinklersStatus[i] < wateringTimeSliderValue) {
//				//        lcd.print(0, 0, "                ");
//				//        String slogan = "#" + i;
//				//        slogan += " ->" + sprinklersStatus[i];
//				//        slogan += "  " + sprinklers[i];
//				String tmp = "#";
//				tmp += i + 1;
//				tmp += " ->";
//				tmp += sprinklersStatus[i];
//				//        tmp += "  ";
//				//        tmp += sprinklers[i];
//				printToLcd(0, tmp);
////				lcd.print(0, 0, "                ");
////				lcd.print(0, 0, tmp);
//
//				sprinklersStatus[i]++;
//				digitalWrite(sprinklers[i], 0);
//				//turn off other valves
//				for (int j = 0; j < 4; j++) {
//					if (j != i) {
//						digitalWrite(sprinklers[j], 1);
//					}
//				}
//				delay(50);
//				break;
//			}
//		}
//		//    lcd.print(6, 1, sprinklersStatus[3]);
//		if (sprinklersStatus[3] >= wateringTimeSliderValue) {
//			routineState = 2;
//			//      finishedSprinklers=0;
//			resetWateringRoutine();
//		}
//		//all sprinklers done
//		//    lcd.print(0, 0, "                ");
//		//        routineState = 0;
//		//    lcd.print(0, 1, "                ");
//		//    lcd.print(0, 0, "Status: FINISH");
//		//    resetWateringRoutine();
//	}
//}

void fillTankWhenWatering() {
	short waterLevel = analogRead(PIN_WATER_SENSOR);
	//tank not full
	//  lcd.print(0, 1, "                ");
	//		if (value_water > 800)
	//	lcd.print(0, 1, "                ");
	//			lcd.print(0, 1, waterLevel);
	if (hydroState == 1 && waterLevel > 800) {
		digitalWrite(PIN_WELL_PUMP, LOW);
		tankFull = false;
		printToLcd(1, "Filling tank...");
//		lcd.print(0, 1, "                ");
//		lcd.print(0, 1, "Filling tank...");
	}
}

void stopWatering() {
	++wateringTime;
	if (hydroState == 1) {
		String tmp = "Time: ";
		tmp += wateringTime;
		printToLcd(0, tmp);
//		lcd.print(0, 0, "                ");
//		lcd.print(0, 0, tmp);
	}

	if (tankFullAtStart) {
		autoStopWateringAfter = wateringTimeSliderValue;
	} else {
		autoStopWateringAfter = 3;
	}

	if (wateringTime > autoStopWateringAfter) {
		//stop hydro
		digitalWrite(PIN_HYDRO, HIGH);
		hydroState = 0;
		Blynk.virtualWrite(VPIN_HYDRO, 0);
	}
}

void checkTankFull() {
	//tank is full
	if (tankFull == false) {
		if (analogRead(PIN_WATER_SENSOR) < 800) {
			printToLcd(1, "Tank full");
			digitalWrite(PIN_WELL_PUMP, HIGH);
			tankFull = true;
			//    timerWell.restartTimer(1);
		}
	}
}

void checkTankFullColdStart() {
	tankFull = false;
	//tank is not full
	checkTankFull();
	if (!tankFull) {
		digitalWrite(PIN_WELL_PUMP, LOW);
		printToLcd(1, "Filling tank...");
		//		lcd.print(0, 1, "Filling tank...");
//		tankFull = false;
	} else {
		printToLcd(1, "Tank full");
		//		lcd.print(0, 1, "Tank full");
//		tankFull = true;
	}
//	if (tankFull == false) {
//		if (analogRead(PIN_WATER_SENSOR) < 800) {
//			printToLcd(1, "Tank full");
//			digitalWrite(PIN_WELL_PUMP, HIGH);
//			tankFull = true;
//			//    timerWell.restartTimer(1);
//		}
//	}
}

void printToLcd(short line, String message) {
	lcd.print(0, line, "                ");
	lcd.print(0, line, message);
}

void setup() {
	// Set console baud rate
	Serial.begin(9600);
	delay(10);
	// Set ESP8266 baud rate
	EspSerial.begin(115200);
	delay(10);
	Blynk.begin(auth, wifi, SSID, PASS);

	pinMode(PIN_WATER_SENSOR, INPUT);
	pinMode(PIN_WELL_PUMP, OUTPUT);
	pinMode(PIN_HYDRO, OUTPUT);
//	pinMode(PIN_S1, OUTPUT);
//	pinMode(PIN_S2, OUTPUT);
//	pinMode(PIN_S3, OUTPUT);
//	pinMode(PIN_S4, OUTPUT);

	//set well and sprinklers pump off by default
	digitalWrite(PIN_WELL_PUMP, HIGH);
	digitalWrite(PIN_HYDRO, HIGH);

	//set default sprinklers time to 0 minutes
//	sprinklersStatus[0] = 0;
//	sprinklersStatus[1] = 0;
//	sprinklersStatus[2] = 0;
//	sprinklersStatus[3] = 0;
//
//	sprinklers[0] = PIN_S1;
//	sprinklers[1] = PIN_S2;
//	sprinklers[2] = PIN_S3;
//	sprinklers[3] = PIN_S4;

	//    Blynk.virtualWrite(V1, wateringTimeSliderValue);
	//    Blynk.virtualWrite(V1, 0);
	unsigned long second = 1000L;
	unsigned long minute = 60 * second;
	unsigned long hour = 60 * minute;

	unsigned long checkTankInterval = 20 * minute;
	unsigned long wateringRoutineInterval = 1 * minute;
	unsigned long stopWateringInterval = minute;

	if (debug) {
		stopWateringInterval = second;
	}

	timer.setInterval(checkTankInterval, checkWaterTank);
	//	timer.setInterval(wateringRoutineInterval, wateringRoutine);
	timer.setInterval(second * 35, fillTankWhenWatering);
	timer.setInterval(stopWateringInterval, stopWatering);
	//check if tank is full
	timer.setInterval(second * checkTankFullInterval, checkTankFull);

	// Setup a function to be called every second
	//  timer.setInterval(1000L, sendSeconds);
	// Setup a function to be called every second
	//  timer.setInterval(1000L, sendMillis);

	while (Blynk.connect() == false) {
		// Wait until connected
	}

	resetWateringRoutine();

//	lcd.clear(); //Use it to clear the LCD Widget
	printToLcd(0, "READY");
	checkTankFullColdStart();
//	lcd.print(0, 0, "                ");
//	lcd.print(0, 0, "READY");
	//  lcd.print(0, 0, ""); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
	//  lcd.print(0, 1, "                ");

//	value_water = analogRead(PIN_WATER_SENSOR);
//	if (value_water > 800) {
//		digitalWrite(PIN_WELL_PUMP, LOW);
//		printToLcd(1, "Filling tank...");
//		tankFull = false;
//	} else {
//		printToLcd(1, "Tank full");
//		tankFull = true;
//	}
}

void loop() {
	Blynk.run();
	timer.run();
}

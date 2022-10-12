//#include "sensitiveInformation.h"
//
//#define FORMAT_SPIFFS_IF_FAILED true
//
//// Wifi & Webserver
//#include "WiFi.h"
//#include "SPIFFS.h"
//#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
//
//AsyncWebServer server(80);
//
//
//// RTC Start - Remove if unnecessary
//#include "RTClib.h"
//
//RTC_PCF8523 rtc;
//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//
//// RTC End
//
//boolean LEDOn = false; // State of Built-in LED true=on, false=off.
//#define LOOPDELAY 100
//
//
//void setup() {
//  Serial.begin(9600);
//  while (!Serial) {
//    delay(10);
//  }
//  delay(1000);
//
//  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
//    // Follow instructions in README and install
//    // https://github.com/me-no-dev/arduino-esp32fs-plugin
//    Serial.println("SPIFFS Mount Failed");
//    return;
//  }
//
//  // Wifi Configuration
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(1000);
//    Serial.println("Connecting to WiFi..");
//  }
//  Serial.println();
//  Serial.print("Connected to the Internet");
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());
//
//  routesConfiguration(); // Reads routes from routesManagement
//
//  server.begin();
//
//  // RTC
//  if (! rtc.begin()) {
//    Serial.println("Couldn't find RTC");
//    Serial.flush();
//    //    abort();
//  }
//
//  // The following line can be uncommented if the time needs to be reset.
//  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  rtc.start();
//  pinMode(LED_BUILTIN, OUTPUT);
//
//}
//
//void loop() {
//
//  builtinLED();
//
//
//  delay(LOOPDELAY); // To allow time to publish new code.
//}
//
//
//void builtinLED() {
//  if (LEDOn) {
//    digitalWrite(LED_BUILTIN, HIGH);
//  } else {
//    digitalWrite(LED_BUILTIN, LOW);
//  }
//}
//
//void logEvent(String dataToLog) {
//  /*
//     Log entries to a file stored in SPIFFS partition on the ESP32.
//  */
//  // Get the updated/current time
//  DateTime rightNow = rtc.now();
//  char csvReadableDate[25];
//  sprintf(csvReadableDate, "%02d,%02d,%02d,%02d,%02d,%02d,",  rightNow.year(), rightNow.month(), rightNow.day(), rightNow.hour(), rightNow.minute(), rightNow.second());
//
//  String logTemp = csvReadableDate + dataToLog + "\n"; // Add the data to log onto the end of the date/time
//
//  const char * logEntry = logTemp.c_str(); //convert the logtemp to a char * variable
//
//  //Add the log entry to the end of logevents.csv
//  appendFile(SPIFFS, "/logEvents.csv", logEntry);
//
//  // Output the logEvents - FOR DEBUG ONLY. Comment out to avoid spamming the serial monitor.
//  //  readFile(SPIFFS, "/logEvents.csv");
//
//  Serial.print("\nEvent Logged: ");
//  Serial.println(logEntry);
//}
#include "sensitiveInformation.h"

#define FORMAT_SPIFFS_IF_FAILED true

#include <Wire.h>



// Wifi & Webserver
#include "WiFi.h"
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);


// RTC Start - Remove if unnecessary
#include "RTClib.h"

RTC_PCF8523 rtc;

// RTC End

// Temperature START

#include "Adafruit_ADT7410.h"
// Create the ADT7410 temperature sensor object
Adafruit_ADT7410 tempsensor = Adafruit_ADT7410();

// Temperature END


// MiniTFT Start
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include "Adafruit_miniTFTWing.h"

Adafruit_miniTFTWing ss;
#define TFT_RST    -1     // we use the seesaw for resetting to save a pin
#define TFT_CS   14       // THIS IS DIFFERENT FROM THE DEFAULT CODE
#define TFT_DC   32       // THIS IS DIFFERENT FROM THE DEFAULT CODE
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);


// MiniTFT End

// Motor Shield START

#include <Adafruit_MotorShield.h>
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(3);

// Motor Shield END

bool fanEnabled = false;            // If the fan is on or off.
bool automaticFanControl = true;    // Automatic or manual control

// ESP32Servo Start

#include <ESP32Servo.h>
Servo myservo;  // create servo object to control a servo
int servoPin = 12;
boolean blindsOpen = false;

// ESP32Servo End

boolean LEDOn = false; // State of Built-in LED true=on, false=off.
#define LOOPDELAY 100


void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }
  delay(1000);

  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    // Follow instructions in README and install
    // https://github.com/me-no-dev/arduino-esp32fs-plugin
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  if (!ss.begin()) {
    Serial.println("seesaw init error!");
    while (1);
  }
  else Serial.println("seesaw started");

  ss.tftReset();
  ss.setBacklight(0x0); //set the backlight fully on

  // Use this initializer (uncomment) if you're using a 0.96" 180x60 TFT
  tft.initR(INITR_MINI160x80);   // initialize a ST7735S chip, mini display

  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  AFMS.begin(); // Motor Shield Start

  // ESP32Servo Start
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000); // attaches the servo on pin 18 to the servo object
  // ESP32Servo End

  Serial.println("ADT7410 demo");

  // Make sure the sensor is found, you can also pass in a different i2c
  // address with tempsensor.begin(0x49) for example
  if (!tempsensor.begin()) {
    Serial.println("Couldn't find ADT7410!");
    while (1);
  }

  // sensor takes 250 ms to get first readings
  delay(250);


  // Wifi Configuration
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println();
  Serial.print("Connected to the Internet");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Display IP on TFT
  tft.setCursor(0, 60);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextWrap(true);
  tft.print(WiFi.localIP());

  routesConfiguration(); // Reads routes from routesManagement

  server.begin();

  // RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    //    abort();
  }
  if (! rtc.initialized() || rtc.lostPower()) {
    logEvent("RTC is NOT initialized, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  rtc.start();


  // MiniTFT Start
  if (!ss.begin()) {
    logEvent("seesaw init error!");
    while (1);
  }
  else logEvent("seesaw started");

  ss.tftReset();
  ss.setBacklight(0x0); //set the backlight fully on

  // Use this initializer (uncomment) if you're using a 0.96" 180x60 TFT
  tft.initR(INITR_MINI160x80);   // initialize a ST7735S chip, mini display

  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  // MiniTFT End

  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {

  builtinLED();
  updateTemperature();
  fanControl();
  delay(LOOPDELAY); // To allow time to publish new code.
}


void builtinLED() {
  if (LEDOn) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void logEvent(String dataToLog) {
  /*
     Log entries to a file stored in SPIFFS partition on the ESP32.
  */
  // Get the updated/current time
  DateTime rightNow = rtc.now();
  char csvReadableDate[25];
  sprintf(csvReadableDate, "%02d,%02d,%02d,%02d,%02d,%02d,",  rightNow.year(), rightNow.month(), rightNow.day(), rightNow.hour(), rightNow.minute(), rightNow.second());

  String logTemp = csvReadableDate + dataToLog + "\n"; // Add the data to log onto the end of the date/time

  const char * logEntry = logTemp.c_str(); //convert the logtemp to a char * variable

  //Add the log entry to the end of logevents.csv
  appendFile(SPIFFS, "/logEvents.csv", logEntry);

  // Output the logEvents - FOR DEBUG ONLY. Comment out to avoid spamming the serial monitor.
  //  readFile(SPIFFS, "/logEvents.csv");

  Serial.print("\nEvent Logged: ");
  Serial.println(logEntry);
}


void tftDrawText(String text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.setTextColor(color, ST77XX_BLACK);
  tft.setTextWrap(true);
  tft.print(text);
}

void updateTemperature() {
  // Read and print out the temperature, then convert to *F
  float c = tempsensor.readTempC();
  float f = c * 9.0 / 5.0 + 32;
  Serial.print("Temp: "); Serial.print(c); Serial.print("*C\t");
  Serial.print(f); Serial.println("*F");
  String tempInC = String(c);
  tftDrawText(tempInC, ST77XX_WHITE);
  delay(100);
}

void automaticFan(float temperatureThreshold) {
  float c = tempsensor.readTempC();
  myMotor->setSpeed(100);
  if (c < temperatureThreshold) {
    fanEnabled = false;
  } else {
    fanEnabled = true;
  }
}

void fanControl() {
  if (automaticFanControl) {
    automaticFan(25.0);
  }
  if (fanEnabled) {
    myMotor->run(FORWARD);
  } else {
    myMotor->run(RELEASE);
  }
}

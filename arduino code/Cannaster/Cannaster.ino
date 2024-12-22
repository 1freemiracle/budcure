#include <Wire.h>          
#include <ESP8266WiFi.h> 
#include <ESP8266WebServer.h>   
#include <SSD1306Wire.h>  
#include <ESPRotary.h>
#include <Ticker.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "FreeMonoBold12pt7b.h"
#include <EEPROM.h>
#include <Stepper.h>

////////////////////////////////////////////////////////server
const char* targetSSID = "G8X"; // Your Wi-Fi SSID
const char* password = "6669inchs"; // Replace with your Wi-Fi password
ESP8266WebServer server(80);

Adafruit_BME280 bme; // I2C
//#define MINUTES_TO_MILLISECONDS(minutes) ((minutes) * 60UL * 1000UL)
#define HOURS_TO_MILLISECONDS(hours)     ((hours) * 60UL * 60UL * 1000UL) 
#define MAX_ADDRESS 0
#define PRESS_ADDRESS 5
#define GO_ADDRESS 10
#define DELAY_ADDRESS 15
#define HUMIDITY_ADDRESS 20
const int stepsPerRevolution = 4060;
const int steps = (stepsPerRevolution/360) * 22;
Stepper myStepper(stepsPerRevolution, D0, D2, D3, D4);
#define relayPin D1
#define ROTARY_PIN1 D5
#define ROTARY_PIN2 D6
#define ENCODER_SWITCH_PIN D7
#define EEPROM_SIZE 4096 

#define CLICKS_PER_STEP 4
#define MIN_POS         0
#define MAX_POS         7
#define START_POS       1
#define INCREMENT       1   
#define SPEEDUP_STEPS   2
ESPRotary r;
Ticker t;
SSD1306Wire *display;
//////////////////////////////////         Rotary Program
volatile bool interruptFlag = false; 
int lastPosition = 1; 
bool moved = false;
int position = 1; 
int debounce = 125; 
///////////////////////////////////        STEPPER MOTOR
int stepsToMove = 375;
int currentStep = 0;
///////////////////////////////////        INTERRUPT
unsigned long x = 0;
void IRAM_ATTR handleInterrupt() {
  if (millis() - x > 1250) {
    interruptFlag = true;
    x = millis();
  }
}
///////////////////////////////////        PROGRAM GO SETUP
int humidityThreshold = 30; 
int pressureThreshold = 5;
unsigned long prgDelay = HOURS_TO_MILLISECONDS(1.5); 
bool enablePrg = true;
bool disablePmp = false;
bool programGo = false;
bool isVacuumOn = false;
unsigned long prgOffTime; 
unsigned long prgStartTime;
unsigned long seconds;
unsigned long minutes;
int maxHumidity = 80;
float humidity;
float pressure;
float psi;
float temperature;
///////////////////////////////////        TIMING FOR MAIN DISPLAY AND SENSOR DATA
int dataLoad = 500; 
unsigned long lastDataReload = 0; 
///////////////////////////////////        EEPROM RECORDING HUMIDITY VALUES DATA POINTS 
static unsigned int lastRecordedMinute = 0;
static int eepromAddress = HUMIDITY_ADDRESS + 10;  
///////////////////////////////////        SPEEDUP FEATURE OF MOTORIZED LID

void setup() {
  pinMode(1, FUNCTION_3);
  pinMode(3, FUNCTION_3);
  pinMode(ENCODER_SWITCH_PIN, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(D0, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  digitalWrite(relayPin, LOW);
  attachInterrupt(digitalPinToInterrupt(ENCODER_SWITCH_PIN), handleInterrupt, FALLING);

  // Display
  Wire.begin(1, 3); //Join I2C bus
  display = new SSD1306Wire(0x3c, 1, 3);
  display->init();
  display->flipScreenVertically();
  display->setFont(ArialMT_Plain_16);  // Set font size
  display->drawString(8, 0, "cannaster dude");
  display->display();
  delay(500);
  unsigned status;
  bme.begin(0x76);
  
  // Rotary encoder
  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  t.attach_ms(3, handleLoop);
  
  // EEPROM
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(MAX_ADDRESS, maxHumidity);
  EEPROM.get(GO_ADDRESS, programGo);
  EEPROM.get(DELAY_ADDRESS, prgDelay);
  EEPROM.get(HUMIDITY_ADDRESS, eepromAddress);
  EEPROM.get(PRESS_ADDRESS, pressureThreshold);

  // STEPPER
  myStepper.setSpeed(7);
  digitalWrite(D0, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);

  WiFi.mode(WIFI_STA); // Set Wi-Fi mode to Station 
  server.on("/", handleRoot);
  server.on("/setVariables", HTTP_POST, handleSetVariables);
  server.on("/data", handleData);
  server.on("/list", handleFileList); 
  server.on("/download", handleDownload);
}

void loop() {
  // ENCODER BUTTON
  if (interruptFlag) {
    delay(debounce);
    interruptFlag = false;
    handleButton(position);
  }
  // TIMED SENSOR UPDATES + UPDATING THE OLED SCREEN
  if ((millis() - lastDataReload) > dataLoad) {
    measurements();
    if (position == 1) {
      displaySensorReadings();
    }
    lastDataReload = millis();
  }
  // WORKING THE ENCODER DIAL ON INTERRUPT
  if (moved) {
    delay(debounce);
    moved = false;
    handleEncoder(position);
  }

////////////////////////////////////////PROGRAM//////////////////////////////////////////////////////////////
  //set the humidity and pressure threshold, and the time delay for the pump to cycle on and off and how long the pump will be on when turned on
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////

  if (programGo) {
    unsigned long currentMillis = millis();  // Capture millis() once for efficiency
    // Calculate elapsed time in minutes and seconds
    unsigned long timeElapsed = currentMillis - prgStartTime;
    seconds = (timeElapsed / 1000) % 60;
    minutes = (timeElapsed / 1000) / 60;
    
    if (minutes % 5 == 0 && minutes != lastRecordedMinute) {
      if (eepromAddress < 4000) {
        // Convert the humidity value to an integer
        int humidityInt = static_cast<int>(humidity);

        // Write the humidity value sequentially to EEPROM
        EEPROM.put(eepromAddress, humidityInt);
        EEPROM.commit();

        // Increment address for the next write
        eepromAddress += sizeof(int);  // Change to sizeof(int) since we're now writing integers

        // Update last recorded minute to avoid multiple writes in the same 5-minute window
        lastRecordedMinute = minutes;
        EEPROM.put(eepromAddress, HUMIDITY_ADDRESS);
        EEPROM.commit();
      }
    }


    // Check if the program should start again after a delay
    if (currentMillis - prgOffTime > prgDelay || humidity > maxHumidity) { //one hour delay
      disablePmp = false; //turns on the pump
      prgStartTime = currentMillis; 
      prgOffTime = currentMillis;
      isVacuumOn = false; 
    }
    
    // Disable the pump if it has been on for too long 
    if (minutes == 5) { 
        disablePmp = true;
    }

    if (!disablePmp) {
      // Turn on the vacuum if conditions are met
      if (!isVacuumOn && humidity > humidityThreshold && psi > pressureThreshold + 1) {
        digitalWrite(relayPin, HIGH); 
        isVacuumOn = true;   
      }

      // Turn off the vacuum when the desired pressure is reached
      if (isVacuumOn && psi < pressureThreshold) {
        digitalWrite(relayPin, LOW); 
        isVacuumOn = false;
      }
    } else {
        // Ensure the pump is off if it's disabled
        digitalWrite(relayPin, LOW);
      }
  } else {
        // Ensure the pump is off if the program is not running
        digitalWrite(relayPin, LOW);
    }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

}



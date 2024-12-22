#include <Wire.h>              
#include "SSD1306Wire.h"       
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ESPRotary.h"
#include "Ticker.h"
#include "DHT_Async.h"
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>  

#define DHT_SENSOR_TYPE DHT_TYPE_22
#define DHT_SENSOR_PIN D7
#define ROTARY_PIN1 D2
#define ROTARY_PIN2 D1
#define ENCODER_SWITCH_PIN D3
#define ONE_WIRE_BUS D4
#define FAN_PIN 16
#define TEC_PIN 15
#define HX711_dout D5 
#define HX711_sck D6
////////////////////////////////////////////////////////server
const char* targetSSID = "G8X"; // Your Wi-Fi SSID
const char* password = "6669inchs"; // Replace with your Wi-Fi password
ESP8266WebServer server(80);

////////////////////////////////////////////////////////loadcell
#define CALIBRATE_ADDRESS 0 //STATIC
#define TARE_ADDRESS 10 //STATIC
#define TRAY_ADDRESS 20 //STATIC
#define FANPROGRAM_ADDRESS 30 //booleen
#define TECPROGRAM_ADDRESS 32 //booleen
#define FANCYCLE_ADDRESS 35 //STATIC
#define FANDURATION_ADDRESS 40 //STATIC 
#define HUMIDITY_ADDRESS 45 //STATIC
#define FROSTY_ADDRESS 50 //STATIC
#define EVENT_ADDRESS 100 //DECIMAL NUMBER
#define TEMP_ADDRESS 300 //STORES AN ARRAY
#define WEIGHT_ADDRESS 200 //STORES AN ARRAY
float calibrationValue; 
int eventCount = 0; //track how many elements are in the weights array

DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
/////////////////////////////////////////////////////////Rotary Encoder
#define CLICKS_PER_STEP 2
#define MIN_POS         0
#define MAX_POS         10
#define START_POS       1
#define INCREMENT       1   
ESPRotary r;
Ticker t;
bool updateHomeScreen = false;  // Flag to trigger home screen update
/////////////////////////////////////////////////////////////Display
SSD1306Wire *display;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = { 0x28, 0x2E, 0x50, 0x57, 0x04, 0xE1, 0x3C, 0x42 };
HX711_ADC LoadCell(HX711_dout, HX711_sck);
/////////////////////////////////////////////////////////////////Weight Program
float loadData; //value from getData()
float trayWeight;
long storedTareValue; 
float weights[99]; 
float temps[99];
float weight;
//////////////////////////////////////////////////////////////////Sensor Data
float temperature; //temp of the box
float humidity;
float humidityThresholdOn = 75.0;  // Humidity level to turn the fan on
float humidityThresholdOff = 65.0; // Humidity level to turn the fan off
float tempC; //temp of the probe
bool flagData = true;
unsigned long seconds;
unsigned long minutes;
unsigned long hours;
//////////////////////////////////////////////////////Rotary Program
volatile bool interruptFlag = false; 
int lastPosition = 1; 
bool moved = false;
int position = 1; //position of the rotary encoder
int debounce = 50; //for the encoder pushbutton

void IRAM_ATTR handleInterrupt() {
  interruptFlag = true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////programs
int fanCyclesPerHour = 4; // Fan turns on this many times per fanCycleInterval
int fanOnDuration;   // length of time in seconds the fan stays on
int fanStartMinute = 0;  // Variable to store the minute when the fan was turned on
int fanCycleInterval;   // Used to calculate time inbetween fan starts
float humidities;
float frosty = -10;
int pwmTec = 0; // for setting the tec mosfet 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////routing 
bool fanGram = false; //toggles in fan display so to keep the fanProgram current in the loop, saved to eeprom in case of reset
bool fanState = false; //used in fan program
bool tecGram = false;

const int MAX_DEVICES = 10;
String devicesOn[MAX_DEVICES];
int deviceCount = 0;

float dewPoint;

void setup() {
  pinMode(1, FUNCTION_3);
  pinMode(3, FUNCTION_3);
  pinMode(ENCODER_SWITCH_PIN, INPUT_PULLUP);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(TEC_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER_SWITCH_PIN), handleInterrupt, FALLING);
  //display
  display = new SSD1306Wire(0x3c, 1, 3);
  display->init();
  display->flipScreenVertically();
  display->setFont(ArialMT_Plain_10);
  //Dallas TEMP sensor
  sensors.begin();
  //Rotary encoder
  r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  t.attach_ms(10, handleLoop);

  SPIFFS.begin();

  EEPROM.begin(512);
  EEPROM.get(TARE_ADDRESS, storedTareValue);
  EEPROM.get(CALIBRATE_ADDRESS, calibrationValue);

  LoadCell.begin();
  unsigned long stabilizingtime = 2000; 
  boolean _tare = false; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);

  LoadCell.setTareOffset(storedTareValue);
  LoadCell.setCalFactor(calibrationValue);

  LoadCell.update();
  LoadCell.refreshDataSet();

  EEPROM.get(EVENT_ADDRESS, eventCount);
  EEPROM.get(TRAY_ADDRESS, trayWeight);
  loadWeights(); //FROM EEPROM

  WiFi.mode(WIFI_STA); // Set Wi-Fi mode to Station 

  server.on("/", handleRoot);
  server.on("/setVariables", HTTP_POST, handleSetVariables);
  server.on("/data", handleData);
  server.on("/list", handleFileList); 
  server.on("/download", handleDownload);

  EEPROM.get(FANPROGRAM_ADDRESS, fanGram);
  EEPROM.get(FANDURATION_ADDRESS, fanOnDuration);
  EEPROM.get(FANCYCLE_ADDRESS, fanCyclesPerHour);
  EEPROM.get(TECPROGRAM_ADDRESS, tecGram);
  EEPROM.get(HUMIDITY_ADDRESS, humidities);
  EEPROM.get(FROSTY_ADDRESS, frosty); 

  if (fanGram) {addDevice("FAN");}
  if (tecGram) {addDevice("TEC");}
  if (!fanCyclesPerHour) {fanCyclesPerHour = 1;}

}


void loop() {
  position = r.getPosition();
  sensors.requestTemperatures();
  tempC = sensors.getTempC(insideThermometer);
  dht_sensor.measure(&temperature, &humidity);
  dewPoint = (temperature - ((100 - humidity) / 5)) - 15;
  if (LoadCell.update()) {
      loadData = LoadCell.getData();
      weight = loadData - trayWeight;
  }
  if (minutes != 0 && (minutes % 5 == 0) && flagData) { 
      logData("data.csv"); 
      flagData = false;  // Prevent further logging in the same 15-minute window
      if (tecGram) {
        logData("data2.csv");
      }
  } 
  else if (minutes % 5 != 0 && !flagData) { 
      flagData = true;  // Reset flag when not in a 15-minute interval
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                                    Encoder Button
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (interruptFlag) {
    delay(debounce);
    switch(position) {
      case 1: 
        wifiScreen();
        break;
      case 2:
        addDevice("TEC");
        tecScreen();
        break;
      case 3:
        newWeightScreen();
        break;
      case 4:
        memoryScreen();
        break;
      case 5:
        resetScreen();
        break;
      case 6:
        calibrationScreen();
        break;
      case 7:
        tareScreen();
        break;
      case 8:
        weightScreen();
        break;
      case 9:
        addDevice("FAN");
        fanScreen();
        break;
        
    }
    
  } 
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                            routing
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  switch (position) {
    case 1: 
      homeScreen();
      break;
  }

  if (moved) {
    moved = false;
    if (position > 1) {
      selectionScreen(position);
    }
  }

  if(tecGram) {
    tecProgram();
  } else {
      removeDevice("TEC");
      digitalWrite(TEC_PIN, LOW);}

  if (fanGram) {
    fanProgram();
  } else {
    removeDevice("FAN");
    digitalWrite(FAN_PIN, LOW);}
  

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                              CLOCK
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

  unsigned long timeElapsed = millis() / 1000;  // Convert to seconds directly
  seconds = timeElapsed % 60;  // Keep seconds [0-59]
  minutes = (timeElapsed / 60) % 60;  // Keep minutes [0-59]
  hours = (timeElapsed / 3600);  // Calculate hours

}

///////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void selectionScreen(int position) {
    const char* options[] = {
  "nanner",
  "Home",
  "TEC",
  "New Weight",
  "Memory",
  "Reset",
  "Calibration",
  "Tare",
  "weight",
  "fan"
  };

  int y = 20;
  int x = 20;
  display->clear();
  for (int i = 2; i < 10; i++) {
    if (i < 6) {
    if (i == position) {
      display->fillRect(0, y, 64, 12);  // Highlight background
      display->setColor(BLACK);
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      display->drawString(0, y, options[i]);  // Highlighted text
      display->setColor(WHITE);
    } else {
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      display->drawString(0, y, options[i]);  // Normal text
    }
    y += 10;
    } else {
      if (i == position) {
      display->setTextAlignment(TEXT_ALIGN_RIGHT);
      display->fillRect(64, x, 128, 12);  // Highlight background
      display->setColor(BLACK);
      display->drawString(128, x, options[i]);  // Highlighted text
      display->setColor(WHITE);
    } else {
      display->setTextAlignment(TEXT_ALIGN_RIGHT);
      display->drawString(128, x, options[i]);  // Normal text
    }
    x += 10;
    }

    
  }
  display->display();

}

void logData(const char* filename) {
    // Check which file to save to based on the keyword
  if (strcmp(filename, "data.csv") == 0) {
   if (!SPIFFS.exists("/data.csv")) {
      File file = SPIFFS.open("/data.csv", "w");  // Create a new file
      if (!file) {
        return;
      }
      // Write the headers
      file.println("Temperature,Humidity,Weight");
      file.close();
    }

    // Open the file for appending the data
    File file = SPIFFS.open("/data.csv", "a");
    if (!file) {
      return;
    }

    file.print(temperature);
    file.print(",");
    file.print(humidity);
    file.print(",");
    file.println(weight);

    file.close();  // Close the file
  } 
  else if (strcmp(filename, "data2.csv") == 0) {
    if (!SPIFFS.exists("/data2.csv")) {
      File file = SPIFFS.open("/data2.csv", "w");  // Create a new file
      if (!file) {
        return;
      }
      // Write the headers
      file.println("probe,box,humidity");
      file.close();
    }

    // Open the file for appending the data
    File file = SPIFFS.open("/data2.csv", "a");
    if (!file) {
      return;
    }

    // Write temperature and humidity data in CSV format
    file.print(tempC);
    file.print(",");
    file.print(temperature);
    file.print(",");
    file.println(humidity);

    file.close();  // Close the file
  } 
  else {
    return; // Exit if the file is not recognized
  }
  
}

static bool measure_environment(float *temp, float *humidity) {
    static unsigned long measurement_timestamp = millis();

    /* Measure once every four seconds. */
    if (millis() - measurement_timestamp > 4000ul) {
        if (dht_sensor.measure(temp, humidity)) {
            measurement_timestamp = millis();
            return (true);
        }
    }

    return (false);
}

void addDevice(String deviceName) {
    if (deviceCount < MAX_DEVICES) {
        devicesOn[deviceCount] = deviceName;
        deviceCount++;
    } else {
        Serial.println("Device limit reached!");
    }
}

void removeDevice(String deviceName) {
    for (int i = 0; i < deviceCount; i++) {
        if (devicesOn[i] == deviceName) {
            // Shift remaining devices to fill the gap
            for (int j = i; j < deviceCount - 1; j++) {
                devicesOn[j] = devicesOn[j + 1];
            }
            devicesOn[deviceCount - 1] = "";  // Clear last entry
            deviceCount--;
            break;
        }
    }
}

bool isDeviceOn(String deviceName) {
    for (int i = 0; i < deviceCount; i++) {
        if (devicesOn[i] == deviceName) {
            return true;
        }
    }
    return false;
}

void displayDevicesOn() {
    display->setTextAlignment(TEXT_ALIGN_LEFT);

    if (deviceCount == 0) {
        display->drawString(0, 54, "None");
    } else {
        // Loop through the devices that are currently on and display them
        for (int i = 0; i < deviceCount; i++) {
            display->drawString(i * 16, 54, devicesOn[i]);
        }
    }
  return;
}

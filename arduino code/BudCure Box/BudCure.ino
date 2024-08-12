#include <Wire.h>              
#include "SSD1306Wire.h"       
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ESPRotary.h"
#include "Ticker.h"
#include "DHT_Async.h"
#include <HX711_ADC.h>
#include <EEPROM.h>

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
////////////////////////////////////////////////////////loadcell
#define TARE_ADDRESS 10
#define WEIGHT_ADDRESS 200
#define EVENT_ADDRESS 100
#define CALIBRATE_ADDRESS 0
#define TRAY_ADDRESS 20
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
float weightLoss; //display loss of moisture content
float trayWeight;
long storedTareValue; 
float weights[100]; 
bool prgStart = false; //starts capturing weight data
//////////////////////////////////////////////////////////////////Sensor Data
float temperature;
float humidity;
float tempC;

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
//////////////////////////////////////////////////////routing
bool fanGram = false;
bool fanState = false;  
bool firstTimeThrough = true; 
String currentProgram; 

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
  loadWeights();

}

void loop() {
  unsigned long currentTime = millis();
  position = r.getPosition();
  sensors.requestTemperatures();
  tempC = sensors.getTempC(insideThermometer);
  dht_sensor.measure(&temperature, &humidity);
  if (LoadCell.update()) {
      loadData = LoadCell.getData();
      weightLoss = (abs(loadData) - abs(trayWeight));
   }
 ////////////////////////////////////////////////////////////////////////////////////////////////

 ///////////////////////////////////Encoder Button///////////////////////////////////////////////
  if (interruptFlag) {
    delay(debounce);
    switch(position) {
      case 1: 
        homeScreen();
        break;
      case 2:
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
        fanScreen();
        break;
        
    }
    
  } 
 ////////////////////////////////////////////////////////////////////////////////////

 ///////////////////////////////routing///////////////////////////////////////////////
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

  if (fanGram) {
    fanProgram();
  } else {
    currentProgram = "NONE";
    digitalWrite(FAN_PIN, LOW);}

  ///////////////////////////////////////////////////////////////////////////////////

 /////////////////////////////////clock///////////////////////////////////////////////

  unsigned long timeElapsed = millis();
  seconds = timeElapsed / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  seconds = seconds % 60;
  minutes = minutes % 60;


}

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


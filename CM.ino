#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include <Encoder.h>
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <MemoryFree.h>
#include <GyverDS18.h>
#include "DHT_Async.h"

#define calVal_eepromAdress 5
#define TARE_ADDRESS 10
#define HX711_dout 9 //mcu > HX711 dout pin
#define HX711_sck 10 //mcu > HX711 sck pin
#define Mosfet 5
#define Transistor 6
#define I2C_ADDRESS 0x3C
#define RST_PIN -1
GyverDS18Single ds(11);
#define DHT_SENSOR_TYPE DHT_TYPE_22
static const int DHT_SENSOR_PIN = 12;

DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
HX711_ADC LoadCell(HX711_dout, HX711_sck);
SSD1306AsciiAvrI2c oled;
Encoder myEnc(3, 4);

float loadData; //DISPLAY value from getData()
float calibrationValue; 
long storedTareValue; //used in setting tare value from menu

float humidity = 0; //DHT
float tempC = 0; //DHT 
float tempF = 0; //sparkfun board
float tTecC = 0; //dallas temp sensor
float tTecF = 0; //dallas temp sensor
float dewPoint = 0;
float oldLoad = 0;
float oldDity = 0;
float oldTempF = 0;

const int encoderCLK = 4; 
const int encoderDT = 3;  
const int encoderSW = 2;  

int programSelect = 1;
int debounceTime = 50; //ENCODER SWITCH
int currentPage = 1; //DISPLAY
int x = 1; //DISPLAY (initial display??)
int pwmValue = 0; //DISPLAY
bool activated = false; //ENGINE
unsigned long t_ = 0; //millis variable for checking on getData() from loadcell

unsigned long lastSensorUpdateTime = 0;
int sensorUpdateInterval = 3000;  // Update sensor readings every 1000 milliseconds
unsigned long offTime_now = 0;
unsigned long onTime_now = 0;
unsigned long fanOnInterval = 3000;
unsigned long fanOffInterval = 10000;


bool _y = true;

void setup() {
  pinMode(encoderCLK, INPUT); 
  pinMode(encoderDT, INPUT);  
  pinMode(encoderSW, INPUT_PULLUP); 
  pinMode(Transistor, OUTPUT);
  pinMode(Mosfet, OUTPUT);
  Serial.begin(9600);                                
  while(Serial == false){};    
  Serial.println("here");                          

  //++++++++++++++++++++++++++++++++++++++++ oled +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  #if RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
  #else // RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
  #endif // RST_PIN >= 0
  oled.setFont(TimesNewRoman13_italic);
  oled.clear();
  oled.println("         ");
  oled.set2X();
  oled.println("BudCure");
  
Serial.println("here");  
  //+++++++++++++++++++++++++++++++++++++++++ Dallas Temperature Sensor +++++++++++++++++++++++++++++++++++++
  ds.requestTemp();
  //++++++++++++++++++++++++++++++++++++++++ load cell +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EEPROM.begin();
  LoadCell.begin();
  EEPROM.get(calVal_eepromAdress, calibrationValue); 
  EEPROM.get(TARE_ADDRESS, storedTareValue);

  Serial.print("Loaded calibration factor: ");
  Serial.println(calibrationValue);
  Serial.print("Loaded tare value: ");
  Serial.println(storedTareValue);
  
  unsigned long stabilizingtime = 2000; 
  boolean _tare = false; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);

 // Check the raw value before setting tare offset
    LoadCell.update();
    Serial.print("Raw value before tare offset: ");
    Serial.println(LoadCell.getData());

    // Set tare offset
    LoadCell.setTareOffset(storedTareValue);

    // Check the raw value after setting tare offset
    LoadCell.update();
    Serial.print("Raw value after tare offset: ");
    Serial.println(LoadCell.getData());

    // Print tare offset to confirm it's set
    Serial.print("Tare offset after setting: ");
    Serial.println(LoadCell.getTareOffset());

  LoadCell.setCalFactor(calibrationValue); 

  LoadCell.update();
  LoadCell.refreshDataSet();

}
long oldPosition  = -999;


void loop() {

////////////////////////////////////////////PROGRAM/////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
dewPoint = (tempC -((100 - humidity)/5)-15);

if (programSelect == 1) {
  programOff();
} else if (programSelect == 2) { 
  fanProgram(); 
  } else if (programSelect == 3) { 
    tecFanProgram();
    } else if (programSelect == 4) {
      tecProgram();
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

////////////////////////////////////////////DISPLAY/////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

  if (millis() - lastSensorUpdateTime >= sensorUpdateInterval) {
    updateSensorReadings();
    if (int(loadData * 10) != int(oldLoad * 10) ||
        int(humidity ) != int(oldDity) ||
        int(tempF) != int(oldTempF) ) {
          
      displaySensorReadings();
      oldLoad = loadData;
      oldTempF = tempF;
      oldDity = humidity;

    }
    lastSensorUpdateTime = millis();
    
  }
 
  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


////////////////////////////////////////////ROTARY ENCODER//////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
  int SWState = digitalRead(encoderSW);
  long newPosition = myEnc.read() / 2; // Divide by 4 for more stable readings
  if (newPosition != oldPosition) {
    delay(50);
  if (newPosition > oldPosition) {
    // Encoder rotated clockwise
    currentPage++;
    if (currentPage > 4) {
      currentPage = 1; // Reset to 1 if it goes above 3
    }
    } else {
      // Encoder rotated counterclockwise
      currentPage--;
      if (currentPage < 1) {
        currentPage = 4; // Set to 3 if it goes below 1
      }
    }
    oldPosition = newPosition;
    handleEncoder(currentPage);
    // Print the currentPage for debugging
    Serial.print("Current Page: ");
    Serial.println(currentPage);
  }
 
  // Button handling
  if (SWState == LOW) {
    delay(debounceTime);
    handleButton(currentPage);
    Serial.print("fuck ");
  }    
  if (currentPage ==1) LoadCell.update();
}

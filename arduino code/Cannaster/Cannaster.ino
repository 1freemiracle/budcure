#include <Encoder.h>
#include <Stepper.h>
#include "DFRobot_AHT20.h"
#include <OLED_I2C.h>
#include "DFRobot_BMP280.h"
#include "Wire.h"

typedef DFRobot_BMP280_IIC    BMP;    // ******** use abbreviations instead of full names ********

BMP   bmp(&Wire, 0x77);

#define SEA_LEVEL_PRESSURE    1015.0f   // sea level pressure


#define MINUTES_TO_MILLISECONDS(minutes) ((minutes) * 60UL * 1000UL)
#define HOURS_TO_MILLISECONDS(hours)     ((hours) * 60UL * 60UL * 1000UL) 
#define encoderSW 4
#define relayPin 5

DFRobot_AHT20 aht20;   
OLED  myOLED(SDA, SCL);

const int stepsPerRevolution = 4060;
Stepper myStepper(stepsPerRevolution, 12, 11, 10, 9);

Encoder myEnc(3, 2);

unsigned long lastDebounceTime = 0;
int debounceTime = 250;
int currentPage = 1;

//program
const int humidityThreshold = 40; // Example threshold in percent
const int pressureThreshold = 7;
unsigned long prgDelay = HOURS_TO_MILLISECONDS(3); //flips the program on and off 
unsigned long pmpDelay = MINUTES_TO_MILLISECONDS(10); //pump turns on once for every program cycle
bool enablePrg = true;
bool disablePmp = false;
bool programGo = false;
bool isVacuumOn = false;
unsigned long prgOffTime;
unsigned long pmpOffTime;
unsigned long prgStartTime;
unsigned long seconds;
unsigned long minutes;

unsigned long lastDataReload = 0;
int dataLoad = 500;
float tempReading;
float humidityReading;
float pressureReading;

extern uint8_t SmallFont[];
extern uint8_t mice5[];
extern uint8_t mice4[];
extern uint8_t mice3[];
extern uint8_t mice2[];
extern uint8_t mice1[];
extern uint8_t mice[];
double vectors[12][2] = {
  {0, 0}, {7, 0}, {7, 7}, {0, 7}, // Box corners
  {0, 0}, {7, 7},                 // Diagonal 1
  {7, 0}, {0, 7},                 // Diagonal 2
};
int xOffset = 113;
int yOffset = 2;


void setup() {
  pinMode(encoderSW, INPUT_PULLUP);  // Set SW pin as input with internal pull-up resistor
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  Serial.begin(9600);
  Wire.begin();

  myOLED.begin(SSD1306_128X64);
  myOLED.setFont(SmallFont);

  bmp.reset();
  myStepper.setSpeed(7);
  bmp.begin();
  aht20.begin();

  startScreen();
}

long oldPosition  = -999;

void loop() {

/////////////////////////////////////////////HOMEPAGE///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

////////////////////////////////////////////ROTARY ENCODER//////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
  int SWState = digitalRead(encoderSW);
  long newPosition = myEnc.read() / 2; // Divide by 4 for more stable readings
  if (newPosition != oldPosition) {
    delay(140);
    if (newPosition > oldPosition) {
      // Encoder rotated clockwise
      currentPage++;
      if (currentPage > 3) {
        currentPage = 1; // Reset to 1 if it goes above 3
      }
    } else {
      // Encoder rotated counterclockwise
      currentPage--;
      if (currentPage < 1) {
        currentPage = 3; // Set to 3 if it goes below 1
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
  }    
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

////////////////////////////////////////PROGRAM//////////////////////////////////////////////////////////////
//set the humidity and pressure threshold, and the time delay for the pump to cycle on and off and how long the pump will be on when turned on
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

if (programGo) {
  //////////////prgDelay is the time between pump pulls
  if (millis() - prgOffTime > prgDelay) {
    enablePrg = !enablePrg;
    disablePmp = false; 
    prgStartTime = millis(); 
    prgOffTime = millis();
    pmpOffTime = millis(); 
    isVacuumOn = false;
  }
  ///////////////
  unsigned long timeElapsed = millis() - prgStartTime;
  seconds = timeElapsed / 1000;
  minutes = seconds / 60;
  seconds = seconds % 60;
  
  /////////////enablePrg turns on the pump once every prgDelay. pmpDelay is the time the pump is on. 
  if (enablePrg) {
    if (millis() - pmpOffTime > pmpDelay) {
      disablePmp = true; // turns off the pump, is reset above in prgOffTime loop
    }

    if (!disablePmp) {
      
      if (!isVacuumOn && humidityReading > humidityThreshold && pressureReading > pressureThreshold +1) {
        digitalWrite(relayPin, HIGH); 
        isVacuumOn = true;   
      }
  
      if (isVacuumOn) {
        if (pressureReading < pressureThreshold) {
          // Turn off the vacuum
          digitalWrite(relayPin, LOW); 
          isVacuumOn = false;
        }
      }
    } else{digitalWrite(relayPin, LOW);}
  } else{digitalWrite(relayPin, LOW);}
} else{digitalWrite(relayPin, LOW);}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

if ((millis() - lastDataReload) > dataLoad) {
  dataReload();
  if (currentPage == 1) {
    displaySensorReadings(tempReading, humidityReading, pressureReading);
  }
  lastDataReload = millis();
}

}

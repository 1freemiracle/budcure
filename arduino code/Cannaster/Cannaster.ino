#include <OLED_I2C.h>
#include <Wire.h>
#include <Encoder.h>
#include <Stepper.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

const int stepsPerRevolution = 4060;
Stepper myStepper(stepsPerRevolution, 12, 11, 10, 9);

#define RESET_PIN  -1  
#define EOC_PIN    -1  
Adafruit_BME280 bme;
Encoder myEnc(3, 2);
OLED  myOLED(SDA, SCL); 

extern uint8_t bitmap[];
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];
extern uint8_t mice5[];extern uint8_t mice4[];extern uint8_t mice3[];extern uint8_t mice2[];extern uint8_t mice1[];extern uint8_t mice[];

unsigned long lastDebounceTime = 0;
int debounceTime = 250;
int currentPage = 1;

//program
const int humidityThreshold = 40; // Example threshold in percent
const int pressureThreshold = 7;
unsigned long prgDelay = 900000; //flips the program on and off 
unsigned long pmpDelay = 300000; //pump turns on once for every program cycle
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
float temperature = 0.0f;
float humidity = 0.0f;
float pressure_hPa = 0.0f;

const int encoderSW = 4;  
const int relayPin= 5;

void setup() {
  pinMode(encoderSW, INPUT_PULLUP);  // Set SW pin as input with internal pull-up resistor
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  Serial.begin(9600);
  while (!Serial) {
      delay(100);
  }

  Wire.begin();
  
  bme.begin(0x76); 

  if(!myOLED.begin(SSD1306_128X64))
    while(1);   // In case the library failed to allocate enough RAM for the display buffer...

  startScreen();

  myOLED.setFont(SmallFont);

  myStepper.setSpeed(7);
  
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
    handleButton();
  }    
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

////////////////////////////////////////PROGRAM//////////////////////////////////////////////////////////////
//set the humidity and pressure threshold, and the time delay for the pump to cycle on and off and how long the pump will be on when turned on
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

if (programGo) {
  Serial.println(disablePmp);
//////////////time switch, its only true once every prgDelay
  if (millis() - prgOffTime > prgDelay) {
    enablePrg = !enablePrg;
    disablePmp = false; //variable reset
    prgStartTime = millis(); 
    prgOffTime = millis();
    pmpOffTime = millis(); //pump starts once each prg cycle
    isVacuumOn = false;
  }
////////////////time telling /////////////////////////////////////

  unsigned long timeElapsed = millis() - prgStartTime;
  seconds = timeElapsed / 1000;
  minutes = seconds / 60;
  seconds = seconds % 60;
  
///////////////////////////////////////////////////////////////////
  if (enablePrg) {
    if (millis() - pmpOffTime > pmpDelay) {
      disablePmp = true; // turns off the pump, is reset above in prgOffTime loop
    }

    if (!disablePmp) {
      
      if (!isVacuumOn && humidity > humidityThreshold && pressure_hPa > pressureThreshold +1) {
        // Turn on the vacuum
        digitalWrite(relayPin, HIGH); 
        isVacuumOn = true;   
      }
  
      if (isVacuumOn) {
        if (pressure_hPa < pressureThreshold) {
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
 displaySensorReadings(temperature, humidity, pressure_hPa);
 }
 lastDataReload = millis();
}
}

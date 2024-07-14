/*
// Display:  SDA pin   -> Arduino Analog 4 or the dedicated SDA pin    blue wire qwiic
//           SCL pin   -> Arduino Analog 5 or the dedicated SCL pin    yellow wire qwiic
printNumF(num, dec,x,y)

#include <EEPROM.h>

int i = 0;
int value = 0;
int address = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Format Starting...");
  delay(500);
}

void loop(){
/*
  //iterate through each byte 
  for (i ; i < EEPROM.length() ; i++) 
  {
    EEPROM.write(i, 0);
    Serial.println(i);
  }
*/
/*
  //reads each address one by one
  value = EEPROM.read(address);

  Serial.print(address);
  Serial.print("\t");
  Serial.print(value, DEC);
  Serial.println();
  address = address + 1;
  if (address == EEPROM.length()) 
  {
    address = 0;
  }
  }
*/

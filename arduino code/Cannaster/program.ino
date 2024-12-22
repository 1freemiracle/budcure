
void humidityChange() {
  int newPosition = position;
  delay(250);

  
  while(!interruptFlag) {
    display->clear();
    display->drawString(8, 14, "Max Humidity:  ");
    display->drawString(45, 38, String(maxHumidity));
    display->display();
  
    if (newPosition != position) {
      if (newPosition > position) {
        // Encoder rotated clockwise
        maxHumidity++;
        if (newPosition - 2 == position) {
          maxHumidity--;
        }
      } else {
          // Encoder rotated counterclockwise
          maxHumidity--;
          if (newPosition + 2 == position) {
            maxHumidity++;
          }
        }
      newPosition = position;
    }  
  }
  delay(debounce);
  EEPROM.put(MAX_ADDRESS, maxHumidity);
  EEPROM.commit();
  interruptFlag = false;
  position = 1;
  r.resetPosition(1);
  handleEncoder(1);

}

void operateLid() {       
  delay(250);
  while (!interruptFlag) {
    display->clear();
    display->drawString(7, 20, "operate moto");
    display->drawString(60, 0, "     ");
    display->display();
    if(moved) {
      byte direction = (byte)r.getDirection();
      String directionStr = String(direction);
      if (directionStr == "1") {
        display->clear();
        display->drawString(50, 0, "forward");
        display->display();
        myStepper.step(-steps);
      }
      if (directionStr == "255") {
        display->clear();
        display->drawString(50, 0, "reverse");
        display->display();
        myStepper.step(steps);
      }
      moved = false;
    }
    
  }
  digitalWrite(D0, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
  delay(250);
  interruptFlag = false;
  position = 1;
  r.resetPosition(1);
  handleEncoder(1);
}

void intervalChange() {
  int newPosition = position;
  delay(250);

  
  while(!interruptFlag) {
    unsigned long hours = prgDelay / 3600000; 
    unsigned long minutes = (prgDelay % 3600000) / 60000;
    display->clear();
    display->setFont(Bold);
    display->drawString(3, 0, "          ");
    display->drawString(37, 27, String(hours));
    display->drawString(55, 27, ": ");
    display->drawString(65, 27, String(minutes));
    display->display();
  
    if (newPosition != position) {
      if (newPosition > position) {
        // Encoder rotated clockwise
        prgDelay += 300000;
        if (newPosition - 2 == position) {
          prgDelay -= 300000;
        }
      } else {
          // Encoder rotated counterclockwise
          prgDelay -= 300000;
          if (newPosition + 2 == position) {
            prgDelay += 300000;;
          }
        }
      newPosition = position;
    }
  }
  delay(debounce);
  EEPROM.put(DELAY_ADDRESS, prgDelay);
  EEPROM.commit();
  interruptFlag = false;
  position = 1;
  r.resetPosition(1);
  handleEncoder(1);

}

void eepromWipe(int startAddress) {
  display->clear();
  display->setFont(ArialMT_Plain_10);
  display->drawString(3, 0, "          ");
  display->drawString(0, 27, "erasing memory");
  display->drawString(57, 27, ".");
  display->display();
  delay(500);

  for (int i = startAddress; i < EEPROM.length(); i++) {
      EEPROM.put(i, 0); 
      EEPROM.commit();
  }

  EEPROM.put(HUMIDITY_ADDRESS, HUMIDITY_ADDRESS + 10);
  EEPROM.commit();
  EEPROM.get(HUMIDITY_ADDRESS, eepromAddress);
  interruptFlag = false;
  position = 1;
  r.resetPosition(1);
  handleEncoder(1);
}



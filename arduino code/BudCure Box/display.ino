///////////////////////////////////////////////////////////////////////////////////      HOMESCREEN
void homeScreen() {
  display->clear();
  display->setFont(ArialMT_Plain_10);
  
  display->drawString(35, 2, "Probe: ");
  display->drawString(55, 2, String(tempC, 1));
  display->drawString(107, 2, "Box: ");
  display->drawString(128, 2, String(temperature, 1));
  display->drawLine(0, 0, 128, 0);
  display->drawLine(0, 15, 128, 15);

  display->drawString(56, 17, "HUMIDITY");
  display->setFont(ArialMT_Plain_16);
  display->drawString(45, 30, String(humidity, 1));
  display->setFont(ArialMT_Plain_10);
  display->drawString(123, 17, "WEIGHT");
  display->setFont(ArialMT_Plain_16);
  display->drawString(120, 30, String(weightLoss));
  display->setFont(ArialMT_Plain_10);

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(1, 54, currentProgram);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  
  display->drawString(128, 54, String(seconds));
  display->drawString(111, 54, ":");
  display->drawString(108, 54, String(minutes));
  display->drawString(96, 54, ":");
  display->drawString(93, 54, String(hours));
  display->display();
}
////////////////////////////////////////////////////////////////////////////////////    NEW WEIGHT SCREEN
void newWeightScreen() {
  interruptFlag = false;
  while (!interruptFlag) {
    LoadCell.refreshDataSet();
    float weight = LoadCell.getData(); 
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 0, "WEIGHT INPUT SCREEN");
    display->drawString(74, 15, String(weight));
    display->display();

    if (weight > 300) {
      display->setFont(ArialMT_Plain_10);
      display->drawString(85, 30, "Remove tray");
      display->display();
    } else {
      display->setFont(ArialMT_Plain_10);
      display->drawString(128, 42, "Press enter NOW...");
      display->display();
    }
  }
  delay(500);
  interruptFlag = false; 
  prgStart = !prgStart; 
  tareTared();
  
}

/////////////////////////////////////////////////////////////////////////////////   TEC SCREEN
void tecScreen() {
  interruptFlag = false;
  while(!interruptFlag) {
    display->clear();
    display->setFont(ArialMT_Plain_16);
    display->drawString(85, 30, "TEC is " + String(analogRead(TEC_PIN)));
    display->display();
  }
  delay(500);
  interruptFlag = false; 
  tecProgram();
}
/////////////////////////////////////////////////////////////////////////////////   FAN SCREEN
void fanScreen() {
  interruptFlag = false;
  fanGram = !fanGram;
  while(!interruptFlag) {
    display->clear();
    display->setFont(ArialMT_Plain_16);
    display->drawString(85, 30, "fan is" + String(fanGram));
    display->display();
  }
  delay(500);
  interruptFlag = false; 
  fanProgram();
}
/////////////////////////////////////////////////////////////////////////////////   RESET SCREEN
void resetScreen() {
  interruptFlag = false;
  while (!interruptFlag) {
    display->clear();
    display->setFont(ArialMT_Plain_16);
    display->drawString(128, 30, "GETT your gun");
    display->display();
  }
  delay(500);
  interruptFlag = false; 
  resetMemory();
}
//////////////////////////////////////////////////////////////////////////////////    CALIBRATION SCREEN
void calibrationScreen() {
  interruptFlag = false;
  while(!interruptFlag) {
    LoadCell.refreshDataSet();
    float weight = LoadCell.getData();
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 0, "CALIBRATE SCREEN");
    display->drawString(100, 14, "remove the tray");
    display->drawString(128, 25,"press enter to continue...");
    display->setFont(ArialMT_Plain_16);
    display->drawString(85, 42, String(weight));
    display->display();
  }
  delay(500);
  interruptFlag = false; 
  calibrate();
}
/////////////////////////////////////////////////////////////////////////////////   TARE SCREEN
void tareScreen() {
  interruptFlag = false;
  while(!interruptFlag) {
    LoadCell.refreshDataSet();
    float weight = LoadCell.getData();
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 0, "CALIBRATE SCREEN");
    display->drawString(100, 14, "remove the tray");
    display->drawString(128, 25,"press enter to continue...");
    display->setFont(ArialMT_Plain_16);
    display->drawString(85, 42, String(weight));
    display->display();
  }
  delay(500);
  interruptFlag = false; 
  tareFairy();
}
///////////////////////////////////////////////////////////////////////////   MEMORY SCREEN
void memoryScreen() {
  interruptFlag = false;
  while(!interruptFlag) {
    display->clear();
    display->setFont(ArialMT_Plain_10);

    // Declare the variables first
    long taring;
    float weighty;
    int eventy;
    float calibraty;
    float trayy;

    // Get values from EEPROM
    EEPROM.get(TARE_ADDRESS, taring);
    EEPROM.get(WEIGHT_ADDRESS, weighty);
    EEPROM.get(EVENT_ADDRESS, eventy);
    EEPROM.get(CALIBRATE_ADDRESS, calibraty);
    EEPROM.get(TRAY_ADDRESS, trayy);

    // Set text alignment and draw strings
    display->clear();
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 2, "Initial Weight: ");
    display->drawString(0, 15, "TARE: ");
    display->drawString(0, 27, "Calibration: ");
    display->drawString(0, 39, "Tray Weight: ");
    display->drawString(0, 53, "Event Count: ");

    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->drawString(128, 2, String(weighty));
    display->drawString(128, 15, String(taring)); 
    display->drawString(128, 27, String(calibraty));
    display->drawString(128, 39, String(trayy));
    display->drawString(128, 53, String(eventy));
    display->display();
  }
  delay(500);
  interruptFlag = false;

  r.resetPosition(1);
  position = 1;
}

///////////////////////////////////////////////////////////////////////////// WEIGHT SCREEN
void weightScreen() {
  interruptFlag = false;
  while(!interruptFlag) {
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 0, "WEIGHT SCREEN");
    displayArray(weights, 8);
  }
  delay(500);
  interruptFlag = false; 
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void displayArray(float* array, int size) {
    
    int y = 14; // Starting Y position for the first line
      for (int i = 0; i < size; i++) {
        if (i < 5) {
          String line = "offset " + String(i + 1) + " :  " + String(array[i], 1); 
          display->drawString(62, y, line); 
          y += 12;
          if (i == 5) {
            y = 14;
          }
        } else {
          String line = "offset " + String(i + 1) + " :  " + String(array[i], 1); 
          display->drawString(128, y, line);
          y += 12; 
        }
      }

    display->display(); 
}

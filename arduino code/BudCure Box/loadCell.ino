void tareTared() {
  /*
  1.remove tray in tareScreen()
  2.tare to zero and record the tare value into TARE_ADDRESS
  3.put the tray with the weight back on the load cell
  4.save the weight to the array and memory (because the memory populates the weight array which i use to display the values)
  */
  //2.
  LoadCell.tare(); 
  long _offset = LoadCell.getTareOffset();
  EEPROM.put(TARE_ADDRESS, _offset);
  EEPROM.commit(); 

  //3.
  float weight = LoadCell.getData(); 
  while (!interruptFlag) {
    LoadCell.refreshDataSet();
    weight = LoadCell.getData(); 
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 0, "WEIGHT INPUT SCREEN");
    display->drawString(105, 15, "replace tray...");
    display->drawString(115, 28, "then press enter");
    display->setFont(ArialMT_Plain_16);
    display->drawString(85, 42, String(weight));
    display->display();
  }
  //4.
  weight = weight - trayWeight; 
  weights[eventCount] = weight; //save to the array
  saveWeights(); //save to the memory pile
  eventCount++;
  EEPROM.put(EVENT_ADDRESS, eventCount);
  EEPROM.commit();
  interruptFlag = false;
  
  r.resetPosition(1);
  position = 1;
}

void calibrate() {
  /*
  1.tray is removed in calibrateScreen()
  2.loadcell is tared to zero
  3.coke is placed on loadcell  
  4.calibraiton set with 453 grams
  5.tray is replaced 
  6.its weight recorded and trayWeight updated
  */
  float weight = LoadCell.getData();
  
  //2.
  LoadCell.tare(); 
  long _offset = LoadCell.getTareOffset();
  EEPROM.put(TARE_ADDRESS, _offset);
  EEPROM.commit(); 
  if (LoadCell.getTareStatus() == true) {
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 0, "CALIBRATION SCREEN");
    display->drawString(128, 35, "TARE COMPLETE");
    display->display();
    delay(700);
  }

  //3.
  while (!interruptFlag) {
    LoadCell.refreshDataSet();
    weight = LoadCell.getData(); 
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 0, "CALIBRATION SCREEN");
    display->setFont(ArialMT_Plain_16);
    display->drawString(85, 14, String(weight));
    display->setFont(ArialMT_Plain_10);
    display->drawString(115, 34, "Place coke on loadcell");
    display->drawString(128, 44, "press enter to continue...");
    display->display();
  }
  delay(500);
  interruptFlag = false;

  //4.
  float known_weight = 453;
  LoadCell.refreshDataSet(); 
  float newCalibrationValue = LoadCell.getNewCalibration(known_weight);
  EEPROM.put(CALIBRATE_ADDRESS, newCalibrationValue);
  EEPROM.commit();
  EEPROM.get(CALIBRATE_ADDRESS, newCalibrationValue);
  display->clear();
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, "CALIBRATION SCREEN");
  display->drawString(128, 24, "new value");
  display->drawString(128, 35, "set at: " + String(newCalibrationValue));
  display->display();

  //5.
  while(!interruptFlag) {
    LoadCell.refreshDataSet();
    weight = LoadCell.getData(); 
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->drawString(128, 0, "CALIBRATION SCREEN");
    display->setFont(ArialMT_Plain_16);
    display->drawString(74, 15, String(weight));
    display->drawString(85, 30, "Remove coke!");
    display->drawString(85, 45, "replace tray");
    display->display();
  }
  delay(500);
  interruptFlag = false; 
  //6.
  EEPROM.put(TRAY_ADDRESS, weight);
  EEPROM.commit();
  EEPROM.get(TRAY_ADDRESS, trayWeight);

  r.resetPosition(1);
  position = 1;
  
}

void tareFairy() {
  long _offset = LoadCell.getTareOffset();
  LoadCell.tare();
  if (LoadCell.getTareStatus()) {
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->drawString(38, 128, "Tare Complete!");
    display->display();
  }
  EEPROM.put(TARE_ADDRESS, _offset);
  EEPROM.commit();
}

// Save weights to EEPROM
void saveWeights() {
  EEPROM.put(WEIGHT_ADDRESS + eventCount* sizeof(float), weights[eventCount]); // Save each weight
  EEPROM.commit();
}

// Load weights from EEPROM
void loadWeights() {
  for (int i = 0; i < eventCount; i++) {
    EEPROM.get(WEIGHT_ADDRESS + i * sizeof(float), weights[i]); // Read each weight
  }
}

// for some reason you have to run the calibration from the example on the first time. I dunno
void loadStatus() {
  if (LoadCell.update()) {
      loadData = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(loadData);
   }
}

void calibration() {
  delay(750);
  
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("press enter to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    int SWState = digitalRead(encoderSW);
    LoadCell.update();
    if (SWState == LOW) {
      delay(debounceTime);
      LoadCell.tareNoDelay();
      }
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place 453 grams on the loadcell.");
  float known_mass = 453;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    int SWState = digitalRead(encoderSW);
    if (SWState == LOW) {
      delay(debounceTime);
      _resume = true;
      }
  }
  float known_weight = 453;
  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  float newCalibrationValue = LoadCell.getNewCalibration(known_weight); //get the new calibration value
  EEPROM.put(calVal_eepromAdress, newCalibrationValue);
  EEPROM.get(calVal_eepromAdress, newCalibrationValue);
  Serial.print("Value ");
  Serial.print(newCalibrationValue);
  Serial.print(" saved to EEPROM address: ");
  Serial.println(calVal_eepromAdress);
}

void manualTare() {
  long _offset = 0;

  LoadCell.tare(); // calculate the new tare / zero offset value (blocking)
  _offset = LoadCell.getTareOffset(); // get the new tare / zero offset value
  EEPROM.put(TARE_ADDRESS, _offset);
  LoadCell.setTareOffset(_offset); // set value as library parameter (next restart it will be read from EEprom)
  Serial.print("New tare offset value:");
  Serial.print(_offset);
  Serial.print(", saved to EEprom adr:");
  Serial.println(TARE_ADDRESS);
}


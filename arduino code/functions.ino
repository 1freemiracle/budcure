void handleEncoder(int currentPage) {
  // Display the current page
  switch (currentPage) {
    case 1:
      displaySensorReadings();
      break;
    case 2:
      displayRelayStatus();
      break;
    case 3:
      displayMemoryInfo();
      break;
    case 4:
      displayCalibration();
      break;
  }
}

void handleButton(int currentPage) {
  // Handle button press
  // Perform actions based on the current page
  switch (currentPage) {
    case 1:
      displaySensorReadings();
      break;
    case 2:
      encoderProgram();
      break;
    case 3:
      manualTare();
      displayMemoryInfo();
      break;
    case 4:
      calibration();
      break;
  }
}

void displaySensorReadings() {
  // first row: TEC status and temp. second is humidity and tempF. thrid: scale data
  oled.set1X();
  oled.setFont(X11fixed7x14);
  oled.clear();
  oled.print("DP: ");
  oled.print(round(dewPoint));
  oled.print("   TEC: ");
  oled.println(round(tTecC));
  oled.setFont(lcdnums12x16);
  oled.print(humidity,1);
  oled.setCol(65);
  oled.println(tempC,1);
  oled.set2X();
  oled.setFont(utf8font10x16);
  oled.setCol(32);
  oled.print(loadData, 1);
}

void displayRelayStatus() {
  // Implement code to display relay status
  Serial.println("Relay Status Page");
  oled.set1X();
  oled.setFont(X11fixed7x14);
  oled.clear();
  switch (programSelect) {
    case 1: 
      oled.println("select program");
      break;
    case 2:
      oled.print("fan on");
      break;
    case 3: 
      oled.println("fan + tec");
      break;
    case 4: 
      oled.println("tec on");
      break;
  }
  oled.setFont(Callibri11_bold);
  oled.set2X();
  oled.setRow(4);
  oled.print("PROGRAMS");
}

void displayMemoryInfo() {
  // Implement code to display memory information
  Serial.println("Memory Info Page");
  oled.set1X();
  oled.setFont(utf8font10x16);
  oled.clear();
  oled.setCol(0);
  oled.setRow(32);
  oled.println(loadData);
  oled.set2X();
  oled.setCol(15);
  oled.setRow(32);
  oled.print("TARE?");
  
}

void displayCalibration() {
  Serial.println("Calibration Page");
  oled.set1X();
  oled.setFont(utf8font10x16);
  oled.clear();
  oled.setCol(0);
  oled.setRow(32);
  oled.println(calibrationValue);
  oled.set2X();
  oled.setCol(15);
  oled.setRow(32);
  oled.print("calibrate?");
}

void updateSensorReadings() 
{
  if (ds.ready()) {         
        if (ds.readTemp()) {  
          tTecC = ds.getTemp();
          Serial.print("TEC: ");
          Serial.print(tTecC);
        } else {
            Serial.println("error");
        }

        ds.requestTemp();  
  }
  
  Serial.print("dew point: ");
  Serial.print(dewPoint);
  Serial.print("  |  ");
  
  
  Serial.print("  |  ");
  Serial.print("pwm: ");
  Serial.println(pwmValue);
  Serial.print("Free memory: ");
  Serial.println(freeMemory()); // Print free memory
  loadStatus();
  Serial.println(digitalRead(Mosfet));

  if (dht_sensor.measure(&tempC, &humidity)) {
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print(" deg. C, H = ");
    Serial.print(humidity, 1);
    Serial.println("%");
  }
}

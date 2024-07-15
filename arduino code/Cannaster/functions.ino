void handleEncoder(int currentPage) {
  // Display the current page
  switch (currentPage) {
    case 1:
      displaySensorReadings(temperature, humidity, pressure_hPa);
      break;
    case 2:
      myOLED.clrScr();
      myOLED.print("open/close lid", LEFT, 15);
      myOLED.print("press enter", CENTER, 32);
      myOLED.update();
      break;
    case 3:
      myOLED.clrScr();
      myOLED.print("turn on vaccum", CENTER, 15);
      myOLED.update();
      break;
  }
}

void handleButton() {

  switch (currentPage) {
    case 1:
      displaySensorReadings(temperature, humidity, pressure_hPa);
      programGo = !programGo;
      prgOffTime = millis();
      pmpOffTime = millis();
      prgStartTime = millis();
      enablePrg = true;
      disablePmp = false;
      isVacuumOn = false;
      break;
    case 2:
      operateLid();
      break;
    case 3:
      digitalWrite(relayPin, !digitalRead(relayPin));
      break;
  }
}

void operateLid() {
  delay(500);
  while (digitalRead(encoderSW) == HIGH) {
    myOLED.clrScr();
    myOLED.print("press enter to exit", LEFT, 16);
    myOLED.update();

  int SWState = digitalRead(encoderSW);
  long newPosition = myEnc.read() / 2; // Divide by 4 for more stable readings
  if (newPosition != oldPosition) {
    if (newPosition > oldPosition) {
      myStepper.step(stepsPerRevolution);
    } else {
      myStepper.step(-stepsPerRevolution);
    }
    oldPosition = newPosition;
  }
  }
  digitalWrite(12, LOW);
  digitalWrite(11, LOW);
  digitalWrite(10, LOW);
  digitalWrite(9, LOW);
  delay(500);
  handleEncoder(2);
}

void displaySensorReadings(float temperature,float humidity,float pressure_hPa) {
  // 
  myOLED.clrScr();
  myOLED.print("Temp", LEFT, 0);
  myOLED.print("RH", CENTER, 0);
  myOLED.print("Pressure", RIGHT, 0);
  myOLED.printNumF(temperature, 2, LEFT, 16);
  myOLED.printNumF(humidity, 2, CENTER, 16);
  myOLED.printNumF(pressure_hPa, 2, RIGHT, 16); // Assuming 2 decimal places for pressure
  if (!programGo) {
    myOLED.print(("Press to start"), LEFT, 50);
  } else {myOLED.print(("       "), LEFT, 50);}
  
  if (programGo) {
    if (enablePrg) {
      myOLED.print("enabled", 10, 35);
    } else {myOLED.print("       ", 10, 35);}
    myOLED.printNumI(minutes, 77, 35);
    myOLED.print(":", 90, 35);
    myOLED.printNumI(seconds, 98, 35);
  }
  myOLED.update();
}

void dataReload() {
 temperature = bme.readTemperature();
 humidity = bme.readHumidity();
 pressure_hPa = (bme.readPressure()/100)/68.9476;
Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Pressure = ");

    Serial.print(pressure_hPa);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.println();
}

void startScreen() {

  myOLED.clrScr();
  myOLED.drawBitmap(0, 0, mice5, 128, 64);
  myOLED.update();
  delay(300);
  myOLED.clrScr();
  myOLED.drawBitmap(0, 0, mice4, 128, 64);
  myOLED.update();
  delay(300);
  myOLED.clrScr();
  myOLED.drawBitmap(0, 0, mice3, 128, 64);
  myOLED.update();
  delay(300);
  myOLED.clrScr();
  myOLED.drawBitmap(0, 0, mice, 128, 64);
  myOLED.update();
  delay(300);
  for (int i=0; i<2; i++)
  {
    myOLED.clrScr();
    myOLED.drawBitmap(0, 0, mice2, 128, 64);
    myOLED.update();
    myOLED.invert(true);
    delay(300);
    myOLED.clrScr();
    myOLED.drawBitmap(0, 0, mice1, 128, 64);
    myOLED.update();
    myOLED.invert(false);
    delay(300);
  }

}
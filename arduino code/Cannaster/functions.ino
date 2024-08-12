void handleEncoder(int currentPage) {
  // Display the current page
  switch (currentPage) {
    case 1:
      displaySensorReadings(tempReading, humidityReading, pressureReading);
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

void handleButton(int currentPage) {
  switch (currentPage) {
    case 1:
      displaySensorReadings(tempReading, humidityReading, pressureReading);
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

void dataReload() {
  //float   alti = bme.calAltitude(SEA_LEVEL_PRESSURE, press);
  pressureReading = (bmp.getPressure()/100)/68.9476;
  if (aht20.startMeasurementReady(/* crcEn = */true)) {
    tempReading = aht20.getTemperature_F();
    humidityReading = aht20.getHumidity_RH();
  }
  Serial.print("Temperature = ");
  Serial.print(tempReading);
  Serial.println(" °C");

  Serial.print("Pressure = ");

  Serial.print(pressureReading);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(humidityReading);
  Serial.println(" %");

  Serial.println();

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



void drawBoxWithX() {
  // Draw the box
  myOLED.drawLine(translateX(vectors[0][0]), translateY(vectors[0][1]), translateX(vectors[1][0]), translateY(vectors[1][1]));
  myOLED.drawLine(translateX(vectors[1][0]), translateY(vectors[1][1]), translateX(vectors[2][0]), translateY(vectors[2][1]));
  myOLED.drawLine(translateX(vectors[2][0]), translateY(vectors[2][1]), translateX(vectors[3][0]), translateY(vectors[3][1]));
  myOLED.drawLine(translateX(vectors[3][0]), translateY(vectors[3][1]), translateX(vectors[0][0]), translateY(vectors[0][1]));

  // Draw the X
  myOLED.drawLine(translateX(vectors[4][0]), translateY(vectors[4][1]), translateX(vectors[5][0]), translateY(vectors[5][1]));
  myOLED.drawLine(translateX(vectors[6][0]), translateY(vectors[6][1]), translateX(vectors[7][0]), translateY(vectors[7][1]));
}

int translateX(double x) {
  return (int)(x + xOffset); // Translate to xOffset
}

int translateY(double y) {
  return (int)(y + yOffset); // Translate to yOffset
}

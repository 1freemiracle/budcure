void encoderProgram() {
  delay(300);
  bool confirmedPressed = false; //flag for while loop
  _y = true;
  while (!confirmedPressed) {
    // +++++++++++++++++++++++++++ use the rotary encoder to switch between display screens+++++++++++++++++++++

    long newPosition = myEnc.read() / 2; // Divide by 4 for more stable readings
    if (newPosition != oldPosition) {
      delay(140);
      _y = true;
      if (newPosition > oldPosition) {
        // Encoder rotated clockwise
        currentPage++;
        if (currentPage > 4) {
          currentPage = 1; // Reset to 1 if it goes above 3
        }
      } else {
        // Encoder rotated counterclockwise
        currentPage--;
        if (currentPage < 1) {
          currentPage = 4; // Set to 3 if it goes below 1
        }
      }

      oldPosition = newPosition;
      Serial.print("Current Page: ");
      Serial.println(currentPage);
    
    }
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (_y) {
      switch (currentPage) {
        case 1:
          turnOff_Display();
          _y = false;
          break;
        case 2:
          fan_Display();
          _y = false;
          break;
        case 3:
          tecFan_Display();
          _y = false;
          break;
        case 4:
          tec_Display();
          _y = false;
          break;
      }
    }
    //+++++++++++++++++++++++++++++++ use the switch to select the program ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Button handling
    int SWState = digitalRead(encoderSW);
    if (SWState == LOW) {
      delay(debounceTime);
      oled.clear();
      if (currentPage == 1) {
        confirmedPressed = true;
        programSelect = 1;
        handleEncoder(2);
        break;
      }else if (currentPage == 2) {  
        confirmedPressed = true;
        programSelect = 2;
        handleEncoder(2);
        break;
      } else if (currentPage == 3) {
        tecFanProgram();
        programSelect = 3;
        offTime_now = millis();
        handleEncoder(2);
        break;
      } else if (currentPage == 4) {
        tecProgram();
        programSelect = 4;
        handleEncoder(2);
        break;
      }
    }
       
  }
  return;
}

void tecProgram() {

/////////////////////////////////////////ENGINE//////////////////////////////////////////////////////////////
// tec is set -15 below the dewpoint. Engine is revved till the difference is within 5 points, then is slowed down. Humidity has to be above 80. 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  if (humidity > 55 && tTecC > -10) {

    float error = tTecC - dewPoint;
    if (error >= 5) {
      analogWrite(Mosfet, 255);
    } else if (error < 5 && error > 1){
    pwmValue = map(error, 0, 10, 100, 255);
    pwmValue = constrain(pwmValue, 0, 255);
    analogWrite(Mosfet, pwmValue);
    } else {analogWrite(Mosfet, 0);}
  } else {analogWrite(Mosfet, 0);}

 return;
}

void fanProgram() {
  if (humidity >= 75) {
  digitalWrite(Transistor, HIGH);
  } else if (humidity < 75) {
    digitalWrite(Transistor, LOW);
  }
  return;
}

void tecFanProgram() {
  if (humidity > 45 && tTecC > -10) {

    float error = tTecC - dewPoint;
    if (error >= 5) {
      analogWrite(Mosfet, 255);
    } else if (error < 5 && error > 1){
    pwmValue = map(error, 0, 10, 100, 255);
    pwmValue = constrain(pwmValue, 0, 255);
    analogWrite(Mosfet, pwmValue);
    } else {analogWrite(Mosfet, 0);}
  } else {analogWrite(Mosfet, 0);}

  if (millis() - offTime_now < fanOffInterval) {
    if (millis() - onTime_now < fanOnInterval) {
      digitalWrite(Transistor, HIGH);
    } else {
      digitalWrite(Transistor, LOW);
    }

  } else {
    offTime_now = millis();
    onTime_now = millis();
  }

 return;
}

void programOff() {
  digitalWrite(Mosfet, LOW);
  digitalWrite(Transistor, LOW);
  return;
}

void tec_Display() {
  oled.set1X();
  oled.setFont(utf8font10x16);
  oled.clear();
  oled.setCol(0);
  oled.setRow(32);
  oled.println("TEC");
  oled.set2X();
  oled.setCol(15);
  oled.setRow(32);
  oled.print("SET?");
}

void fan_Display() {
  oled.set1X();
  oled.setFont(utf8font10x16);
  oled.clear();
  oled.setCol(0);
  oled.setRow(32);
  oled.println("FAN");
  oled.set2X();
  oled.setCol(15);
  oled.setRow(32);
  oled.print("SET?");
}

void tecFan_Display() {
  oled.set1X();
  oled.setFont(utf8font10x16);
  oled.clear();
  oled.setCol(0);
  oled.setRow(32);
  oled.println("TEC+");
  oled.set2X();
  oled.setCol(15);
  oled.setRow(32);
  oled.print("SET?");
}

void turnOff_Display() {
  oled.set1X();
  oled.setFont(utf8font10x16);
  oled.clear();
  oled.setCol(0);
  oled.setRow(32);
  oled.println("OFF");
  oled.set2X();
  oled.setCol(15);
  oled.setRow(32);
  oled.print("SET?");
}

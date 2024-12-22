/*
this program calculates the dew point and uses the resulting error between measurement and calcuated value to set the pwm for the TEC. A frosty temp is used to adjust to prevent frost buildup. 
*/
void tecProgram() {
  float celsius = (temperature - 32) * 5.0 / 9.0;
  if (humidity > humidities && tempC > frosty) {

    float error = tempC - dewPoint;
    if (error >= 5) {
      analogWrite(TEC_PIN, 255);
    } else if (error < 5 && error > 1){
    pwmTec = map(error, 0, 10, 100, 255);
    pwmTec = constrain(pwmTec, 0, 255);
    analogWrite(TEC_PIN, pwmTec);
    } else {analogWrite(TEC_PIN, 0);}
  } else {analogWrite(TEC_PIN, 0);}

}
/*
This program starts the fan every "fanCycleInterval" (x times in an hour) for length of time "fanOnDuration" (in minutes).
*/
void fanProgram() {
  // Calculate the interval between fan cycles in minutes
  fanCycleInterval = 60 / fanCyclesPerHour;

  if ((minutes - fanStartMinute) >= fanCycleInterval && !fanState) {
    // Time to turn on the fan
    digitalWrite(FAN_PIN, HIGH);
    fanState = true;
    fanStartMinute = minutes;  // Record the minute the fan was turned on
  }

  // Turn off the fan after the specified duration (in minutes)
  if (fanState && (minutes - fanStartMinute >= fanOnDuration)) {
    digitalWrite(FAN_PIN, LOW);
    fanState = false;
  }
}

void resetMemory() {
  for (int i = 30; i < 512; i++) {
        EEPROM.write(i, 0); 
    }
    
    // Commit changes to EEPROM
    EEPROM.commit();
    eventCount = 0;

    for (int i = 0; i < 99; i++) {
      weights[i] = 0;
    }
    SPIFFS.remove("/data.csv");
    SPIFFS.remove("/data2.csv");
    delay(500);
    r.resetPosition(1);
    position = 1;
}

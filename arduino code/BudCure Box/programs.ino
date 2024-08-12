void tecProgram() {
  digitalWrite(TEC_PIN, !analogRead(TEC_PIN));
  delay(500);
  tareScreen();
}

void fanProgram() {
  currentProgram = "FAN";
  if (firstTimeThrough && humidity > 75) {
    digitalWrite(FAN_PIN, HIGH);  // Turn on the fan on the first run if humidity is above 75%
    fanState = true;              // Update the fan state
    firstTimeThrough = false;     // Mark that the first time through is done
  } else if (humidity > 77 && !fanState) {
    digitalWrite(FAN_PIN, HIGH);  // Turn on the fan if humidity exceeds 77%
    fanState = true;              // Update the fan state
  } else if (humidity < 75 && fanState) {
    digitalWrite(FAN_PIN, LOW);   // Turn off the fan if humidity falls below 75%
    fanState = false;             // Update the fan state
  }
}



void resetMemory() {
  for (int i = 20; i < 512; i++) {
        EEPROM.write(i, 0); 
    }
    
    // Commit changes to EEPROM
    EEPROM.commit();
    eventCount = 0;

    for (int i = 0; i < 100; i++) {
      weights[i] = 0;
    }

    r.resetPosition(1);
    position = 1;
}

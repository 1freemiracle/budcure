void handleEncoder(int position) {
  unsigned long hours = prgDelay / 3600000; 
  unsigned long minutes = (prgDelay % 3600000) / 60000;
  switch (position) {
    case 1:
      displaySensorReadings();
      break;
    case 2:
      display->clear();
      display->setFont(Bold);
      display->drawString(3, 0, "operate motors");
      display->display();
      break;
    case 3:
      display->clear();
      display->setFont(Bold);
      display->drawString(3, 0, "change max rH");
      display->drawString(45, 27, String(maxHumidity));
      display->display();
      break;
    case 4:
      display->clear();
      display->setFont(Bold);
      display->drawString(3, 0, "change delay");
      display->drawString(37, 27, String(hours));
      display->drawString(55, 27, ": ");
      display->drawString(65, 27, String(minutes));
      display->display();
      break;
    case 5:
      display->clear();
      display->setFont(Bold);
      display->drawString(3, 0, "CLEAR MEMORY");
      display->drawString(37, 14, String(eepromAddress / sizeof(int)));
      display->drawString(55, 27, "OF");
      display->drawString(65, 45, String(3950 / sizeof(int)));
      display->display();
      break;
  }
}
void handleButton(int position) {
  switch (position) {
    case 1:
      programGo = !programGo;
      EEPROM.put(GO_ADDRESS, programGo);
      EEPROM.commit();
      prgOffTime = millis();
      prgStartTime = millis();
      disablePmp = false;
      isVacuumOn = false;
      displaySensorReadings();
      break;
    case 2:
      operateLid();
      break;
    case 3:
      humidityChange();
      break;
    case 4:
      intervalChange();
      break;
    case 5:
      eepromWipe(20);
      break;
    case 6:
      wifiScreen();
      break;
  }
}

void wifiScreen() {
    server.begin();
    //wl_status_t status = WiFi.status();
    interruptFlag = false;
    unsigned long lastScanTime = 0;  // Timer for Wi-Fi scan
    const unsigned long scanInterval = 1000;  // 10-second interval between scans

    while (!interruptFlag) {
        wl_status_t status = WiFi.status();
        display->clear();
        display->setFont(ArialMT_Plain_10);
        display->drawString(128, 0, "WIFI SCREEN");

        // Show the current Wi-Fi status and IP address
        if (status == WL_CONNECTED) {
            display->drawString(120, 15, "Status: Connected");
            display->drawString(120, 30, "IP: " + WiFi.localIP().toString());
            server.handleClient();  // Handle web server requests when connected
        } else {
            display->drawString(120, 15, "Status: Not Connected");

            // Only scan for networks every 10 seconds
            if (millis() - lastScanTime > scanInterval) {
                display->drawString(100, 14, "Scanning...");

                int n = WiFi.scanNetworks();  // Scan for available networks
                bool found = false;

                for (int i = 0; i < n; ++i) {
                    display->drawString(128, 25 + i * 10, String(WiFi.SSID(i))); // Display SSID
                    if (WiFi.SSID(i) == targetSSID) {
                        display->drawString(128, 40, "Connecting to " + String(targetSSID));
                        WiFi.begin(targetSSID, password);  // Attempt to connect

                        // Wait for the connection to establish
                        unsigned long connectStartTime = millis();
                        while (WiFi.status() != WL_CONNECTED && millis() - connectStartTime < 10000) {
                            display->clear();
                            display->setFont(ArialMT_Plain_10);
                            display->drawString(128, 0, "Connecting to " + String(targetSSID));
                            
                            // Display a message indicating the waiting status
                            display->drawString(128, 20, "Please wait...");
                            
                            // Update the display
                            display->display();
                            delay(500);  // Wait for a while before checking again
                        }

                        found = true;
                        break;
                    }
                }

                if (!found) {
                    display->drawString(128, 50, "SSID not found");
                }

                lastScanTime = millis();  // Update the scan timer
            }
        }
        
        // Update the display once at the end
        display->display();
        yield();  // Allow background tasks to run to prevent watchdog resets
        delay(500);
    }
    server.stop();
    delay(500);  // Debounce delay
    interruptFlag = false; 
    r.resetPosition(1);
    position = 1;
}

void displaySensorReadings() {
  float fahrenheit = (temperature * 9.0 / 5.0) + 32.0;
  display->clear();
  display->setFont(ArialMT_Plain_10);
  display->drawString(10, 2, "RH");
  display->drawString(50, 2, "temp");
  display->drawString(90, 2, "press");
  display->drawString(10, 16, String(humidity));
  display->drawString(50, 16, String(fahrenheit));
  display->drawString(90, 16, String(psi));
  delay(50);
  if (!programGo) {
    display->drawString(30, 30, "press to start");
    display->drawString(10, 35, "          ");
    display->display();
  } 
  if (programGo) {
    display->drawString(10, 35, "enabled");
    display->drawString(77, 35, String(minutes));
    display->drawString(90, 35, ":");
    display->drawString(98, 35, String(seconds));
    display->display();
  }
  
}
void measurements() {

  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F; 
  psi = pressure * 0.0145038;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void rotate(ESPRotary& r) {
  position = r.getPosition();
  if (position != lastPosition) {
    moved = true;
    lastPosition = position;
  }
  if (position == 7) {
    r.resetPosition(1);
    position = 1;
  } else if (position == 0) {
      r.resetPosition(6);
      position = 6;
    }
}
/////////////////////////////////////////////////////////////////
void showDirection(ESPRotary& r) {
  //Serial.println(r.directionToString(r.getDirection()));
}
/////////////////////////////////////////////////////////////////
void handleLoop() {
  r.loop();
}
/////////////////////////////////////////////////////////////////

/*
5 pin encoder wiring: the three pins go clk - gnd - dt
and the two other pins go: hook up to microcontroller - gnd
good luck
*/

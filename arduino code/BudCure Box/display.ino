///////////////////////////////////////////////////////////////////////////////////      WIFISCREEN
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


////////////////////////////////////////////////////////////////////////////////////    HOMESCREEN
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
  display->drawString(120, 30, String(weight));
  display->setFont(ArialMT_Plain_10);

  displayDevicesOn();
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
  delay(500);
  interruptFlag = false; 
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
  tecGram = !tecGram;
  EEPROM.put(TECPROGRAM_ADDRESS, tecGram);
  EEPROM.commit();
  interruptFlag = false; 
  delay(500);
  r.resetPosition(1);
  position = 1;
}
/////////////////////////////////////////////////////////////////////////////////   FAN SCREEN
void fanScreen() {
  interruptFlag = false;
  while(!interruptFlag) {
    display->clear(); // Clear the display
    display->setFont(ArialMT_Plain_16);
    
    // Read the fan pin value and ensure correct type for display
    int fanValue = analogRead(FAN_PIN);
    String displayString = "FAN is " + String(fanValue);
    
    // Use drawString with proper arguments
    display->drawString(85, 30, displayString);
    display->display(); // Update the display

    delay(100); // Add a small delay for stability
  }
  fanGram = !fanGram;
  EEPROM.put(FANPROGRAM_ADDRESS, fanGram);
  EEPROM.commit();
  interruptFlag = false; 
  delay(500);
  r.resetPosition(1);
  position = 1;
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
    display->drawString(128, 0, "TARE SCREEN");
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
    displayArrays(weights, temps, 8, 8);
  }
  delay(500);
  interruptFlag = false; 
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void displayArrays(float* array1, float* array2, int size1, int size2) {
    
    int yLeft = 14;  // Starting Y position for the first column (left side)
    int yRight = 14; // Starting Y position for the second column (right side)
    
    // Display the first array (left side)
    for (int i = 0; i < size1; i++) {
        if (i < 5) { // Limit the number of rows to fit the display
            String line = "Offset " + String(i + 1) + ": " + String(array1[i], 1); 
            display->drawString(62, yLeft, line);  // Display on the left side (x = 62)
            yLeft += 12; // Increment Y position for next line
        }
    }

    // Display the second array (right side)
    for (int i = 0; i < size2; i++) {
        if (i < 5) { // Limit the number of rows to fit the display
            String line = "Offset " + String(i + 1) + ": " + String(array2[i], 1); 
            display->drawString(128, yRight, line);  // Display on the right side (x = 128)
            yRight += 12; // Increment Y position for next line
        }
    }

    display->display(); // Refresh the display to show the updates
}


void handleRoot() {
    String html = 
    "<!DOCTYPE html>"
    "<html lang='en'>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<title>ESP8266 Web Server</title>"
    "<style>"
    "body {"
    "    font-family: Arial, sans-serif;"
    "    background-color: #f4f4f4;"
    "    color: #333;"
    "    text-align: center;"
    "    margin: 0;"
    "    padding: 20px;"
    "}"
    "h1 {"
    "    color: #007BFF;"
    "}"
    "p {"
    "    font-size: 1.2em;"
    "    margin: 10px 0;"  // Adjust spacing around paragraphs
    "}"
    "label {"
    "    display: block;"
    "    margin: 10px 0 5px;  // Add space above the input fields"
    "}"
    "input {"
    "    padding: 10px;"
    "    margin: 5px 0;"  // Adjust spacing between inputs
    "    width: calc(100% - 22px);"
    "    max-width: 300px;"
    "    border: 1px solid #ccc;"
    "    border-radius: 5px;"
    "    box-shadow: 1px 1px 3px rgba(0,0,0,0.1);"
    "}"
    "button {"
    "    display: block;"  // Make the button a block element
    "    padding: 10px;"
    "    margin: 10px auto;"  // Center the button and add margin above and below
    "    background-color: #007BFF;"
    "    color: white;"
    "    border: none;"
    "    cursor: pointer;"
    "    transition: background-color 0.3s;"
    "    width: 100%;"
    "    max-width: 300px;"
    "    border-radius: 5px;"
    "}"
    "button:hover {"
    "    background-color: #0056b3;"
    "}"
    "a {"
    "    display: inline-block;"
    "    margin-top: 20px;"
    "    padding: 10px 15px;"
    "    background-color: #007BFF;"
    "    color: white;"
    "    text-decoration: none;"
    "    border-radius: 5px;"
    "    transition: background-color 0.3s;"
    "}"
    "a:hover {"
    "    background-color: #0056b3;"
    "}"
    "</style>"
    "</head>"
    "<body>"
    "<h1>Welcome to the ESP8266 Web Server</h1>"
    "<p>Use the links below to interact with your device.</p>"
    "<p>Current Dew Point: <strong>" + String(dewPoint) + " °C</strong></p>"
    "<form action='/setVariables' method='POST'>"
    "<label for='fanCyclesPerHour'>Fan Cycles Per Hour:</label>"
    "<input type='number' id='fanCyclesPerHour' name='fanCyclesPerHour' min='1' value='" + String(fanCyclesPerHour) + "'>"
    "<label for='fanOnDuration'>Fan On Duration (minutes):</label>"
    "<input type='number' id='fanOnDuration' name='fanOnDuration' min='1' value='" + String(fanOnDuration) + "'>"
    "<label for='frosty'>Minimum TEC Temp:</label>"
    "<input type='number' id='frosty' name='frosty' min='1' value='" + String(frosty) + "'>"
    "<label for='humidities'>Humidity Cutoff:</label>"
    "<input type='number' id='humidities' name='humidities' min='1' value='" + String(humidities) + "'>"
    "<button type='submit'>Set Variables</button>"
    "</form>"
    "<a href='/data'>View Sensor Data</a>"
    "</body>"
    "</html>";

    server.send(200, "text/html", html);
}



void handleSetVariables() {
    if (server.hasArg("fanCyclesPerHour")) {
        fanCyclesPerHour = server.arg("fanCyclesPerHour").toInt();
        EEPROM.put(FANCYCLE_ADDRESS, fanCyclesPerHour);
        EEPROM.commit();
    }
    if (server.hasArg("fanOnDuration")) {
        fanOnDuration = server.arg("fanOnDuration").toInt();
        EEPROM.put(FANDURATION_ADDRESS, fanOnDuration);
        EEPROM.commit();
    }
    if (server.hasArg("frosty")) {
      frosty = server.arg("frosty").toInt();
      EEPROM.put(FROSTY_ADDRESS, frosty);
      EEPROM.commit();
    }
    if (server.hasArg("humidities")) {
      humidities = server.arg("humidities").toInt();
      EEPROM.put(HUMIDITY_ADDRESS, humidities);
      EEPROM.commit();
    }
    server.send(200, "text/html", "<h1>Variables Updated!</h1><p><a href='/'>Back</a></p>");
}


void handleData() {
  // Open the first file
  File file1 = SPIFFS.open("/data.csv", "r");
  if (!file1) {
    server.send(404, "text/plain", "File 1 not found");
    return;
  }

  // Open the second file
  File file2 = SPIFFS.open("/data2.csv", "r");
  if (!file2) {
    server.send(404, "text/plain", "File 2 not found");
    file1.close();  // Close the first file if the second fails to open
    return;
  }

  // Read the content of both files into a string
  String fileContent;

  // Read content of the first file
  while (file1.available()) {
    fileContent += char(file1.read());
  }
  file1.close();  // Close the first file

  // Add a separator between files (optional)
  fileContent += "\n\n-- Next File --\n\n";

  // Read content of the second file
  while (file2.available()) {
    fileContent += char(file2.read());
  }
  file2.close();  // Close the second file

  // Send the combined content as plain text
  server.send(200, "text/plain", fileContent);
}


void handleFileList() {
    String html = "<html><body><h1>SPIFFS Files</h1><ul>";
    Dir dir = SPIFFS.openDir("/");
    
    // Loop through all files and create download links
    while (dir.next()) {
        String fileName = dir.fileName();
        size_t fileSize = dir.fileSize();
        
        // Add the file name and size to the HTML
        html += "<li><a href='/download?file=" + fileName + "'>" + fileName + "</a> (" + String(fileSize) + " bytes)</li>";
    }
    
    html += "</ul></body></html>";
    
    // Send the HTML page
    server.send(200, "text/html", html);
}
void handleDownload() {
    // Get the file name from the query parameter
    String fileName = server.arg("file");
    
    // Ensure that the file name starts with '/'
    if (!fileName.startsWith("/")) {
        fileName = "/" + fileName;
    }

    // Open the requested file
    File file = SPIFFS.open(fileName, "r");
    if (!file) {
        server.send(404, "text/plain", "File not found");
        return;
    }

    // Set the headers to suggest a filename for download
    String contentDisposition = "attachment; filename=" + fileName;
    server.sendHeader("Content-Disposition", contentDisposition);
    
    // Stream the file back to the client
    server.streamFile(file, "application/octet-stream");
    file.close();
}


void displaySensorReadings(float temperature,float humidity,float pressure) {
  myOLED.clrScr();
  myOLED.print("  RH", LEFT, 2);
  myOLED.print(" temp", CENTER, 2);
  drawBoxWithX();
  myOLED.print(String(humidityReading), LEFT, 17);
  myOLED.print(String(tempReading), CENTER, 17);
  myOLED.print(String(pressureReading), RIGHT, 17);
  if (!programGo) {
    myOLED.print("press to start", CENTER, 50);
  } else {
    myOLED.print("_______", CENTER, 50);
  }

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

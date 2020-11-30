//Libraries from github//
#include <Arduino.h>
#include <M5Atom.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//original libraries//
#include "debug_test.h"

//Declaration of github libraries or ESP32's original libraries// 
BluetoothSerial BTSerial;
//Declaration of my original libraries// 
DebugMathNP::DebugMath dm;

String GetFileFromSPIFFS(const char* path){
  String result;

  return result;
}
void BackgroundProcess(void *parameter){
  while(1){
    delay(5000);
  }
  return;
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  BTSerial.begin("Whatever");

  Serial.println("ESP32_ready");
//  xTaskCreatePinnedToCore(BackgroundProcess,"core0-1",1024,NULL,1,NULL,0);
}

void loop() {
  Serial.printf("debug math lib :");
  Serial.println(dm.Add(4,4));
  Serial.println("Hello World");
  delay(1000);
}
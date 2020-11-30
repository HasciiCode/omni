/*
メモ
・mDNSでドメインの名前を追加する
*/

#include <Arduino.h>
#include <M5Stack.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "debug_test.h"

#define SYSTEM_PATH "/device/network.json"

StaticJsonDocument<512> sys_info;
BluetoothSerial BTSerial;
AsyncWebServer server(80);

//*****************************************************************************************//
String GetFileFromSPIFFS(const char* path){
  String result;
  File fr = SPIFFS.open(path,"r");
  result = fr.readString();
  fr.close();

  return result;
}

//*****************************************************************************************//
void APServerMode(const char* ap_ssid,const char* ap_pass){
  WiFi.softAP(ap_ssid,ap_pass);
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World it's");
  });

  //IPアドレスだけ入力するとメインページに飛ぶ
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });

  //linkでstyle.cssにアクセスしたときにCSSファイルをロードする。
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.begin();
}

//*****************************************************************************************//
void BackgroundProcess(void *parameter){
  while(1){

    delay(5000);
  }
  return;
}

//*****************************************************************************************//
void setup() {
  const char* device_name = "";
  const char* sta_ssid = "";
  const char* sta_pass = "";
  const char* ap_ssid = "ESP32-failed-spiffs";
  const char* ap_pass = "none-pass";

  Serial.begin(115200);
  
  if(SPIFFS.begin(true)){
    Serial.println("SPIFFS_is_ready");
    Serial.println("Loading system data...");
    String system_json = GetFileFromSPIFFS(SYSTEM_PATH);
    deserializeJson(sys_info,system_json);

    device_name = sys_info["device"]["name"];
    ap_ssid = sys_info["wifi"]["ap_ssid"];
    ap_pass = sys_info["wifi"]["ap_pass"];
    sta_ssid = sys_info["wifi"]["sta_ssid"];
    sta_pass = sys_info["wifi"]["sta_pass"];
  }else{
    Serial.println("SPIFFS_is_failed");
  }

  BTSerial.begin(device_name);
  APServerMode(ap_ssid,ap_pass);

  Serial.printf("sta_ssid:");
  Serial.println(sta_ssid);
  Serial.printf("sta_pass:");
  Serial.println(sta_pass);

  WiFi.begin(sta_ssid,sta_pass);

  for (int i = 0; i<5;i++){
    if(WiFi.status() == WL_CONNECTED){
      break;
    }
    delay(1000);
  }

  if(WiFi.isConnected()){
    Serial.println("WiFi_is_connected");
  }else{
    Serial.println("WiFi_is_not_connected");
  }

  Serial.println("ESP32_ready");
//  xTaskCreatePinnedToCore(BackgroundProcess,"core0-1",1024,NULL,1,NULL,0);
}

//*****************************************************************************************//
void loop() {
  delay(1000);
}
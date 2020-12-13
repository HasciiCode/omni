/*
memo
  - when you want access to local server you have to put ".local" after domain name.
  - StatiJsonDocument can not use more than one.
  - When you use serialize() you have to initialize String value before use the function again.
*/

#include <Arduino.h>            //LGNU
#include <M5Stack.h>            //MIT
#include <SPIFFS.h>             //Apache
#include <ArduinoJson.h>        //MIT
#include <BluetoothSerial.h>    //Apache
#include <WiFi.h>               //LGNU
#include <ESPmDNS.h>            //MIT
#include <AsyncTCP.h>           //LGNU
#include <ESPAsyncWebServer.h>  //LGNU

#define SYSTEM_PATH "/sys/device.json"
#define NETWORKSCAN_RESULT "/web/api/network.csv"

StaticJsonDocument<1024> device_info;
BluetoothSerial BTSerial;
AsyncWebServer server(80);

//*****************************************************************************************//
void WriteFileOnSPIFFS(const char* path,String data){
  File fw = SPIFFS.open(path,"w");
  fw.print(data);
  fw.close();
  return;
}
//*****************************************************************************************//
String ReadFileFromSPIFFS(const char* path){
  String result;
  File fr = SPIFFS.open(path,"r");
  result = fr.readString();
  fr.close();

  return result;
}
//*****************************************************************************************//
//This function scans SSIDs that ESP32 is able to connect
String NetworkScan(){
  String result;
  byte ssid_num = WiFi.scanNetworks();
  
  for (int i=0; i<ssid_num; i++){
    //Serial.println(WiFi.SSID(i));
    if (i == 0){
      result = WiFi.SSID(i);
    }else{
      result = result + "," + WiFi.SSID(i);
    }
  }
  return result;
}
//*****************************************************************************************//
//Server code that will not intrupt process on main loop
void Setup_Server(const char* server_name){
  //Use DNS instead of IP address
  if(MDNS.begin(server_name)){
    Serial.printf(server_name);
    Serial.println(".local_is_available");        //IPではなく名前で表示される。末尾に.localを入れる必要がある。
  }else{
    Serial.println("local_server_is_available");  //DNSに失敗してIPで表示される
  }

  //for debug use
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World it's ESP32");
  });

  //accessing this url to jump to main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/web/main.html", String(), false);
  });

  //Front process of regist-sta.html
  server.on("/regist-sta.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/web/regist-sta.html", String(), false);
  });
  
  //Background process of regist-sta.html
  server.on("/sta_info.regist", HTTP_POST, [](AsyncWebServerRequest * request){
    int params = request->params();
    bool device_restart = false;
    String serialize_data;

    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        const char* name = p->name().c_str();
        const char* value = p->value().c_str();
        String str_name = String(name);
        String str_value = String(value);
        
        Serial.printf("POST[%s]: %s\n", name, value);

        if(str_name == "sta-ssid"){
          device_info["sta"]["ssid"] = value;
        }else if(str_name == "sta-pass"){
          device_info["sta"]["pass"] = value;
        }else if(str_name == "server-url"){
          device_info["server"]["url"] = value;
        }else if(str_name == "reset"){
          if(str_value == "true"){
            device_restart = true;
          }else{
            device_restart = false;
          }
        }else{
          Serial.println("New Post data:");
          Serial.printf("POST[%s]: %s\n", name, value);
        }
      }
    }
    
    request -> send(200);

    serializeJson(device_info,serialize_data);
    //Serial.println(serialize_data);
    WriteFileOnSPIFFS(SYSTEM_PATH,serialize_data);
    serialize_data = "\0";

    if(device_restart){
      esp_restart();
    }
  });

  //load css for interface design
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/web/style.css", "text/css");
  });

  //accessing to URL to return network list with csv format.
  server.on("/api/network.csv", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/sys/network.csv", "text/csv");
  });



  //when you need somethig to regist, send to /registration with URL to save datas
  server.on("/registration", HTTP_POST, [](AsyncWebServerRequest * request){
    int params = request->params();
    Serial.printf("POST recieved. params: ");
    Serial.println(params);

    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){ //p->isPost() is also true
        Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request -> send(200);
    esp_restart();
  });

  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void LCD_SF(bool sf = true){
  m5.Lcd.setTextColor(ILI9341_WHITE);
  m5.Lcd.printf("[");
  if(sf == true){
    m5.Lcd.setTextColor(ILI9341_GREEN);
    m5.Lcd.printf("SUCCESS");
  }else{
    m5.Lcd.setTextColor(ILI9341_RED);
    m5.Lcd.printf("FAILED");
  }
  m5.Lcd.setTextColor(ILI9341_WHITE);
  m5.Lcd.println("]");
}
//*****************************************************************************************//
//Back ground loop that saves device infomation datas to spiffs
void BackgroundProcess(void *parameter){
  while(1){
    WriteFileOnSPIFFS(NETWORKSCAN_RESULT,NetworkScan());
    delay(5000);
    //Serial.println(ReadFileFromSPIFFS(NETWORKSCAN_RESULT));
    delay(5000);
  }
  return;
}

void Setup_WiFI(){
  m5.Lcd.fillScreen(ILI9341_BLACK);
  m5.Lcd.setTextSize(1);

  String device_json = ReadFileFromSPIFFS(SYSTEM_PATH);
  deserializeJson(device_info,device_json);
  const char* ap_ssid = device_info["ap"]["ssid"];
  const char* ap_pass = device_info["ap"]["pass"];
  const char* sta_ssid = device_info["sta"]["ssid"];
  const char* sta_pass = device_info["sta"]["pass"];
  const char* server_name = device_info["server"]["domain_name"];

  m5.Lcd.printf("AP Mode:");
  if(WiFi.softAP(ap_ssid,ap_pass)){
    LCD_SF();
  }else{
    LCD_SF(false);
  }

  m5.Lcd.println("AP IP address: 192.168.4.1");

  if(device_info["sta"]["connection"] == "true"){
    WiFi.begin(sta_ssid,sta_pass);
    m5.Lcd.printf("WiFi conneting to ");
    m5.Lcd.println(sta_ssid);

    for(int i=0;i<5;i++){
      if(WiFi.status() == WL_CONNECTED){
        break;
      }
      delay(1000);
    }
    Serial.println();
    Setup_Server(server_name);
    Serial.println("server_is_ready");

    m5.Lcd.printf("STA Mode: ");

    String server_url = String("http://") + server_name +String(".local"); 
    if(WiFi.isConnected()){

      //String server_url = String("http://") + String(WiFi.localIP().toString());

      LCD_SF();      
      m5.Lcd.printf("STA IP address: ");
      m5.Lcd.println(WiFi.localIP());
      m5.Lcd.setTextColor(ILI9341_GREEN);
      m5.Lcd.println("Web Application will work with both AP and STA mode");
      m5.Lcd.setTextColor(ILI9341_BLACK);
      Serial.println("WiFi_is_connected");
      M5.Lcd.qrcode(server_url,180,100,140,6);
    }else{
      LCD_SF(false);
      m5.Lcd.setTextColor(ILI9341_YELLOW);
      m5.Lcd.println("Web Application will work with only AP mode");
      m5.Lcd.setTextColor(ILI9341_BLACK);
      M5.Lcd.qrcode(server_url,180,100,140,6);
      Serial.println("WiFi_is_not_connected");
    }
  }
}
//*****************************************************************************************//
//Setup WiFi, Bluetooth Serial, Server, strages
void setup() {
  const char* bt_name = "";

  M5.begin();  
  if(SPIFFS.begin(true)){
    Serial.println("SPIFFS_is_ready");
  }else{
    Serial.println("SPIFFS_is_failed");
  }

  Setup_WiFI();

  WriteFileOnSPIFFS(NETWORKSCAN_RESULT,NetworkScan());
  BTSerial.begin(bt_name);

  Serial.println("ESP32_ready");
  xTaskCreatePinnedToCore(BackgroundProcess,"core0-1",4096,NULL,25,NULL,0);
}
//*****************************************************************************************//
void loop() {

  if(WiFi.isConnected()){
    Serial.println("[=]WiFi is connected");
  }else{
    Serial.println("[!]WiFi is NOT connected");
  }
  delay(10000);
}
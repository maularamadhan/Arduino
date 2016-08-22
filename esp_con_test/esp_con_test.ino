#include <ESP8266.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define charSize 100
#define eeAddress 0

ESP8266 wifi(Serial1, 115200);
String ssid = "Tritronik Mobile";
String password = "Tri12@11";
char c_json[charSize];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  wifi.leaveAP();
  
  build_eeprom_mem();
  read_n_joinAP(read_eeprom_mem()); 

  Serial.println("done!");

}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial1.available()) {
      Serial.write(Serial1.read());
  }
  while (Serial.available()) {
      Serial1.write(Serial.read());
  }

}

void build_eeprom_mem(void)
{
  /* build the eeprom memory */
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["ssid"] = ssid;
  data["password"] = password;
  
  String jsonin;
  data.printTo(jsonin);
  strncpy(c_json, jsonin.c_str(), charSize);
  c_json[charSize - 1] = '\0';
  Serial.println(c_json);
  EEPROM.put(eeAddress, c_json);
}

char* read_eeprom_mem(void)
{
  /* read the built eeprom memory */
  EEPROM.get(eeAddress, c_json);
  Serial.println(c_json);
  String jsonout(c_json);
  char* charjson;
  charjson = strdup(jsonout.c_str());
  return charjson;
}

bool read_n_joinAP(char* data_read)
{
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data_read);
  if (root.success())
  {
    if (wifi.joinAP(root["ssid"],root["password"])){
      Serial.println("Join success!");
      return true;
    }
  }
  Serial.println("error");
  return false;
}


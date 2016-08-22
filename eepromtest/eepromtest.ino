#include <ArduinoJson.h>
#include <EEPROM.h>

String ssid = "Tritronik";
String password = "Tri12@11";
char* c_json;

int eeAddress = 0;

void setup() {
  Serial.begin(115200);
  EEPROM.get(eeAddress, c_json);
  Serial.print("data read1-> ");Serial.println(c_json);
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["ssid"] = ssid;
  data["password"] = password;
  
  String json;
  data.printTo(json);
  c_json = strdup(json.c_str());
  Serial.print("data written-> ");Serial.println(c_json);
  //EEPROM.put(eeAddress, c_json);
  c_json = "";
  Serial.print("data zero-> ");Serial.println(c_json);
  EEPROM.get(eeAddress, c_json);
  Serial.print("data read-> ");Serial.println(c_json);
  Serial.print("data read2-> ");Serial.println(c_json);
}

void loop() {
  // put your main code here, to run repeatedly:

}

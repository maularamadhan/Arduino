#include <ArduinoJson.h>

#define NUMBER_OF_SHIFT_REG  5

byte SWITCH_ARRAYS[NUMBER_OF_SHIFT_REG];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    //wait serial port
  }

 
}

void loop() {
  // put your main code here, to run repeatedly:

}



#define ESP8266 Serial1;
#define SIM900 Serial;

boolean connect_via_wifi()
{
 jsonBuffer.clear();
 StaticJsonBuffer<200> jsonBuffer;
 JsonObject& data = jsonBuffer.createObject();
 data["cmd"] = "auth";
 data["mode"] = "opr";
 data["uid"] = uid;
 data["hmac"] = hmac_sim900;
 data["via"] = "wifi";

 data.printTo(ESP8266);
}

boolean connect_via_gprs()
{
 jsonBuffer.clear();
 StaticJsonBuffer<200> jsonBuffer;
 JsonObject& data = jsonBuffer.createObject();
 data["cmd"] = "auth";
 data["mode"] = "opr";
 data["uid"] = uid;
 data["hmac"] = hmac_sim900;
 data["via"] = "gprs";

 data.printTo(SIM900);
}

void data_sending (byte arrays[])
{
  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& data = jsonBuffer.createObject();
  JsonArray& SW = data.createNestedArray("SWITCH");
  for (int i = 0; i < NUMBER_OF_SHIFT_REG; i++)
  {
    SW.add(SWITCH_ARRAYS[i]);
  }

  data.printTo(Serial);
}


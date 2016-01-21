#include <ArduinoJson.h>
#include <MD5.h>

#define NUMBER_OF_SHIFT_REG  6
#define ESP8266 Serial1
#define SIM900 Serial

byte SWITCH_ARRAYS[NUMBER_OF_SHIFT_REG];

// Carrier for Commands
const char* cmd;
const char* cmd_type;
const char* cmd_id;
int         shift;
int         shift_bit;
int         shift_on;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    //wait serial port
  }
  connect_via_wifi();
  Serial.print("done!");
 
}

void loop() {
  // put your main code here, to run repeatedly:

}

boolean connect_via_wifi(void)
{
 unsigned char* hash=MD5::make_hash("HA-opr-wifi-gbtw");
 char *hmac_esp8266 = MD5::make_digest(hash, 16);
 free(hash);

 StaticJsonBuffer<256> jsonBuffer;
 JsonObject& data = jsonBuffer.createObject();
 data["cmd"] = "auth";
 data["mode"] = "opr";
 data["uid"] = "gbtw";
 data["hmac"] = hmac_esp8266;
 data["via"] = "wifi";

 String json;
 Serial.println("coba");
 data.printTo(json);
 Serial.println(json);
 Serial.println(data.measureLength());
 Serial.println(json.length());
 Serial.println();
 Serial.println("hohooo");
}

/*boolean connect_via_gprs()
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
}*/

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

void command_parsing(uint8_t buffer[128])
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)buffer);
  
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return;
  }
  
  cmd       = root["cmd"];
  cmd_type  = root["type"];
  cmd_id    = root["id"];
  shift     = root["shift"];
  shift_bit = root["bit"];
  shift_on  = root["on"];
  
}

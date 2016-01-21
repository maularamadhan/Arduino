#include <ArduinoJson.h>
#include <MD5.h>
#include <doxygen.h>
#include <ESP8266.h>

#define NMAX_SHIFTREG 25
int n_shiftreg;

uint8_t current_driver[NMAX_SHIFTREG];
uint8_t SWITCH_ARRAYS[NMAX_SHIFTREG];

ESP8266 wifi(Serial1, 115200);

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}


bool auth_reply(uint8_t buffer[128])
{
  //Auth_reply detected
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)buffer);
  
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return false;
  }

  n_shiftreg = what_n_shiftreg(root);
  
  if (parsing_auth_reply(root) && send_current_init(root))
  {
    return true;
  }
  return false;
 
  
}

bool command_control(uint8_t buffer[128])
{
  //command detected
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)buffer);
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return false;
  }

  execute_control(root["shift"], root["bit"], root["value"]);

  if (notify_cmd_success (root["cmd"], root["type"], root["id"]))
  {
    return true;
  }
  return false;
}

bool keep_alive(void)
{
  //ping detected
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["cmd"] = "pong";

  if(send_message(data))
  {
    return true;
  }
  return false;
}

bool update_panel_status(int n_shiftreg)
{
  //current_driver extracted
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["cmd"] = "current_update";
  JsonArray& current_panel = data.createNestedArray("current_panel");
  for (int i=0; i<n_shiftreg; i++)
  {
    current_panel.add(SWITCH_ARRAYS[i]); 
  }
  
  if(send_message(data))
  {
    uint8_t buffer[128] = {0};
    recvFromServer(buffer);
    if(parse_n_check(buffer,"status"))
    {
      return true;
    }
    return false;
  }
  return false;
}

bool notify_cmd_success (const char* cmd, const char* type, const char* id)
{
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["cmd"]     = cmd;
  data["type"]    = type;
  data["id"]      = id;
  data["status"]  = "success";

  if(send_message(data))
  {
    //success
    return true;
  }
  return false;
}

void execute_control(int shift, int shift_bit, int value)
{
  bitWrite(current_driver[shift], shift_bit, value);
}

int what_n_shiftreg(JsonObject& root)
{
  return root["current_driver"].size();
}

bool parse_n_check(uint8_t buffer[128], const char* cmd_class)
{
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)buffer);
  
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return false;
  }

  if (root.containsKey(cmd_class))
  {
    if (cmd_class == "status")
    {
        if (root[cmd_class] == "success"){
          return true;
        }
        return false;
    }
    return true;
  }
  return false;
}

bool parsing_auth_reply(JsonObject& root)
{
  if (root["status"] == "success")
  {
    for (int i=0; i<root["current_driver"].size(); i++)
    {
      current_driver[i]  = root["current_driver"][i];
    }
    return true;
  } else {
    return false;
  }
}

bool send_current_init(JsonObject& root)
{
  //current_driver extracted
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["cmd"] = "current_init";
  JsonArray& current_panel = data.createNestedArray("current_panel");
  for (int i=0; i<root["current_driver"].size(); i++)
  {
    current_panel.add(SWITCH_ARRAYS[i]); 
  }
  
  if(send_message(data))
  {
    uint8_t buffer[128] = {0};
    recvFromServer(buffer);
    if(parse_n_check(buffer,"status"))
    {
      return true;
    }
    return false;
  }
  return false;
}

bool send_message(JsonObject& data)
{
  String json;
  data.printTo(json);
  json = json + "\r\n";
  const char* charjson;
  charjson = json.c_str();
  if(wifi.send(0, (const uint8_t*)charjson, strlen(charjson)))
  {
   Serial.println("Send OK!");
   return true;
  } else {
   Serial.println("Send not OK!");
   return false;
  }
}

void recvFromServer (uint8_t buffer[128])
{
  static uint8_t mux_id = 0;
  uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 10000);
    /*if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
    }*/
}


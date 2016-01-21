#include <doxygen.h>
#include <ArduinoJson.h>
#include <MD5.h>
#include <ESP8266.h>

#define siESP Serial1
#define SIM900 Serial

#define NMAX_SHIFTREG 25
int n_shiftreg;

uint8_t current_driver[NMAX_SHIFTREG];
uint8_t SWITCH_ARRAYS[NMAX_SHIFTREG];
uint32_t lenceu;

#define SSID        "Tritronik Mobile"
#define PASSWORD    "Tri12@11"
#define ServerName  "192.168.130.111"
#define ServerPort  (9123)

ESP8266 wifi(Serial1, 115200);
//ESP8266 wifi2(Serial, 115200);

// Carrier for Commands
bool commandDetected = false;
bool is_wifi_connected = false;
bool is_server_connected = false;
bool init_is_done = false;
uint8_t buffer[128];

/* Driver Write & Read */
//HARDWARE CONNECTIONS
//Pin connected to ST_CP of 74HC595
int latchPin = 17;
//Pin connected to SH_CP of 74HC595
int clockPin = 16;
////Pin connected to DS of 74HC595
int dataPin = 14;
//Pin connected to MR of 74HC595
int resetPin = 15;

/* Selecting Mode */
// Define what pin to determine mode & what value to carry them
int inPin1 = A3;
int STATEA = 0;

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
    STATEA = digitalRead(inPin1);   // read the input pin
    Serial.print(STATEA);
    if (STATEA == 0)
    {
          startSMARTCONFIG();
    }
    /*// MAKE M
    SWITCH_ARRAYS[0] = 0;//24;
    SWITCH_ARRAYS[1] = 0;//198;
    SWITCH_ARRAYS[2] = 0;//153;
    SWITCH_ARRAYS[3] = 0;//36;
    SWITCH_ARRAYS[4] = 0;//33;
    SWITCH_ARRAYS[5] = 0;//231;
    /*/// MAKE W
    SWITCH_ARRAYS[0] = 60;
    SWITCH_ARRAYS[1] = 201;
    SWITCH_ARRAYS[2] = 36;
    SWITCH_ARRAYS[3] = 152;
    SWITCH_ARRAYS[4] = 192;
    SWITCH_ARRAYS[5] = 199;

    SERIAL_OUT_SETUP();
    
    //startSMARTCONFIG();
    wifi.releaseTCP(0);
    while (!auth_via_wifi())
    {
      wifi.leaveAP();
    }
    recvFromServer(buffer);
    auth_reply(buffer);
}
void loop()
{
    /*while (Serial1.available()) {
        Serial.write(Serial1.read());
    }*/
    /*while (Serial.available()) {
        Serial1.write(Serial.read());
    }*/
    if (!init_is_done)
    {
      wifi.releaseTCP(0);
      while (!auth_via_wifi())
      {
        wifi.leaveAP();
      }
      recvFromServer(buffer);
      auth_reply(buffer);
      init_is_done=true;
    }
    if (commandDetected) {
      if (init_is_done){
       recv_cmd();
       commandDetected = false;
      }
    }
}

void startSMARTCONFIG(void)
{
    Serial.print("Setup Starting...\n");
    Serial.println("Turning on SMARTCONFIG...");
    Serial1.begin(115200);
    delay(1000);
    if (wifi.SMARTCONFIG()){
      Serial.println("SMART-ON!");
    } else{
      Serial.println("SMART FAILED!"); 
    }
    Serial.println("Waiting for esptouch...");
    while (!wifi.findSMARTEND())
    {
      // do wait...

    }
    Serial.println("Ready to go!");
}

bool ESPconnect(void)
{
  init_is_done = false;
  Serial1.begin(115200);
  if (wifi.setOprToStation() && wifi.sATCWDHCP(1,1))
  {
    wifi.enableMUX(); // mux enabled
    if (wifi.ConnectAPCheck()){
      Serial.println("Still Connected!");
      return true;
    } else {
      Serial.println("Reconnecting...");
      return wifi.joinAP(SSID, PASSWORD);
    }
  }
}

bool ConnecttoServer(void)
{
  // Note : Has connected to AP or GSM Network
  if (wifi.createTCP(0, ServerName, ServerPort))
  {
    Serial.print("create tcp...");
    Serial.println("ok");
  } else {
    Serial.println("create tcp...err");
  }
  //delay(2000);
}

bool auth_via_wifi(void)
{
 if (!ESPconnect())
 {
  return false;
 }
 if (!ConnecttoServer())
 {
  return false;
 }
 recvFromServer(buffer);
 if (!parse_n_check(buffer, "cmd", "auth"))
 {
  return false;
 }
 
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
 data.printTo(json);
 json = json + "\r\n";
 const char* charjson;
 charjson = json.c_str();
 //Serial.print(charjson);
 if(wifi.send(0, (const uint8_t*)charjson, strlen(charjson)))
 {
  Serial.println("Send OK!");
  return true;
 } else {
  Serial.println("Send not OK!");
  return false;
 }
 Serial.print(charjson);
 Serial.println(strlen(charjson));
}

void recv_cmd (void)
{
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)buffer);

  if (!root.success())
  {
    return;
  }
  Serial.println("asik bisa euuy");
  
  if (check_cmd_n_content(root,"cmd","command"))
  {
    command_control(root);
    Serial.println("command_mode");
  }

  if (check_cmd_n_content(root,"cmd","ping"))
  {
    keep_alive();
    Serial.println("pinging mode");
  }
  
}

void serialEvent1()
{
  while (Serial1.available()) {
    recvFromServer(buffer);
    if (wifi.isTCPClosed())
    {
      init_is_done = false;
      Serial.println("Akhirnya..");
    }
    commandDetected = true;
  }
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
  if (!root.containsKey("current_driver"))
  {
    wifi.releaseTCP(0);
    init_is_done = false; 
    return false;
  }

  if (!parsing_auth_reply(root))
  {
    return false;
  }

  if(!send_current_init(root))
  {
    return false;
  }

  recvFromServer(buffer);
  if(!parse_n_check(buffer,"status","success"))
  {
    Serial.println("poor man you!");
    return false;
  }
  init_is_done = true;
  Serial.println("great man can stand!");
  return true;
  //what_n_shiftreg(root);
}

bool command_control(JsonObject& root)
{
  //command detected
  const char* cmd = root["cmd"];
  const char* cmd_type = root["type"];
  const char* cmd_id = root["id"];
  int cmd_shift = root["shift"];
  int cmd_bit = root["bit"];
  int cmd_on = root["on"];

  Serial.println(cmd);
  Serial.println(cmd_type);
  Serial.println(cmd_id);
  Serial.println(cmd_shift);
  Serial.println(cmd_bit);
  Serial.println(cmd_on);
  
  Serial.println("lanjut bang!");
  execute_control(root["shift"], root["bit"], root["on"]);

  if (notify_cmd_success (root["cmd"], root["type"], root["id"]))
  {
    write_shift_regs(current_driver);
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
    if(parse_n_check(buffer,"status","success"))
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


bool parse_n_check(uint8_t buffer[128], const char* cmd_class, const char* value)
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
     const char* check = root[cmd_class];
     if ((strcmp (check,value) == 0))
     {
      return true;
     }
     return false;
  }
}

bool check_cmd_n_content(JsonObject& root, const char* cmd_class, const char* value)
{
  if (root.containsKey(cmd_class))
  {
     const char* check = root[cmd_class];
     if ((strcmp (check,value) == 0))
     {
      return true;
     }
     return false;
  }
}

bool parsing_auth_reply(JsonObject& root)
{
  if (root["current_driver"].size() != 0)
  {
    n_shiftreg = root["current_driver"].size();
    for (int i=0; i<root["current_driver"].size(); i++)
    {
      current_driver[i]  = root["current_driver"][i];
    }
    /*for (int j=0; j < root["current_driver"].size(); j++)
    {
      Serial.println(current_driver[j]);
    }*/
    return true;
  } else {
    Serial.println("duh sini deh");
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
  send_message(data);
  return true;
}

bool send_message(JsonObject& data)
{
  String json;
  data.printTo(json);
  json = json + "\r\n";
  const char* charjson;
  charjson = json.c_str();
  //Serial.println(charjson);
  if(wifi.send(0, (const uint8_t*)charjson, strlen(charjson)))
  {
   Serial.println("Send OK!");
   return true;
  } else {
   Serial.println("Send not OK!");
   return false;
  }
}

void recvFromServer (uint8_t buffer1[128])
{
  memset(buffer1, 0, sizeof(buffer1));

  uint8_t buffer[128];
  static uint8_t mux_id = 0;
  uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 10000);
  
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
            buffer1[i] = buffer[i];
        }
        Serial.print("]\r\n");
    }
    lenceu = len;
}

void write_shift_regs(uint8_t outcoming[NMAX_SHIFTREG])
{
  digitalWrite(latchPin, LOW); //ground latchPin and hold low as long as transmitting
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram
  for (int i=0; i < n_shiftreg; i++)
  {
    Serial.println(outcoming[i]);
    shiftOut(dataPin, clockPin, MSBFIRST, outcoming[(n_shiftreg-1)-i]);
    //all data is shifting...
  }
  digitalWrite(latchPin, HIGH); //return the latchPin to signal the chip and change the Driver state all at once
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram
}

void SERIAL_OUT_SETUP(void)
{
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
}

uint8_t bit_inverse(uint8_t byte_to_invert)
{
  return (255-byte_to_invert);
}

/*
void printbuffer (uint8_t buffer1[128],uint32_t lent)
{
  buffer1[128] = {0};
  uint8_t buffer[128];
  static uint8_t mux_id = 0;
  uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 10000);
  lent = len;
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
            buffer1[i] = buffer[i];
        }
        Serial.print("]\r\n");
    }
}*/


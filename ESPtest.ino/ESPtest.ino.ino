#include <ArduinoJson.h>
#include <MD5.h>
#include <ESP8266.h>
#include <Timer.h>
#include <EEPROM.h>

#define NMAX_SHIFTREG 25
int n_shiftreg=10;

Timer t;

uint8_t current_driver[NMAX_SHIFTREG];
uint8_t SWITCH_ARRAYS[NMAX_SHIFTREG];
uint32_t lenceu;

//#define ServerName  "128.199.208.149"
#define ServerName  "192.168.130.111"
#define ServerPort  (9123)

// Self-reset
#define selfreset A5

//eeprom memory carrier
#define eeAddress 0
#define charSize 100

//ssid & password stored in eeprom
char c_json[charSize];

ESP8266 wifi(Serial1, 115200);
//SIM900 SIM900(Serial2, 115200);

// Carrier for Commands
bool commandDetected = false;
bool init_is_done = false;
uint8_t buffer[256];

// Connection Status Timeout Variable
int timeout_counter;
int addtimerEvent;
int con_attempt=0;
int addconattemptEvent;

/* Driver Write & Read */
//HARDWARE CONNECTIONS
//Pin connected to OE of 74HC595
int oePin = 2;
//Pin connected to ST_CP of 74HC595
int latchPin = 4;
//Pin connected to SH_CP of 74HC595
int clockPin = 5;
////Pin connected to DS of 74HC595 
int dataPin = 3;
//Pin connected to MR of 74HC595
int resetPin = 6;

// Connect the following pins between your Arduino and the 74HC165 Breakout Board
const int data_pin = 10; // Connect Pin 10 to SER_OUT (serial data out)
const int shld_pin = 7; // Connect Pin 7 to SH/!LD (shift or active low load)
const int clk_pin = 9; // Connect Pin 9 to CLK (the clock that times the shifting)
const int ce_pin = 8; // Connect Pin 8 to !CE (clock enable, active low)

/* Selecting Mode */
// Define what pin to determine mode & what value to carry them
int inPin1 = A3;
int STATEA = 0;

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
    setup_selfreset();

    //Wait until smartconfig done & switch state change
    whileSMARTCONFIG();
    /*****************/
    t.every(5000, routine);
    initialize_conn();
}
void loop()
{
    check_timeout_disconnection();
    if (!init_is_done)
    {
      //Serial.println("--> masuk init_is_done=false");
      wifi.releaseTCP();
      if(auth_via_wifi()){
        //Serial.println("--> masuk auth_via_wifi");
        recvFromServer(buffer);
        auth_reply(buffer);
        enable_outputs();
      }
     }
    if (commandDetected) {
      timeout_counter = 0;
      if (init_is_done){
       recv_cmd();
       commandDetected = false;
      }
    }
    t.update();
}

bool startSMARTCONFIG(void)
{
    String ssid;
    String password;
    Serial.print("Setup Starting...\n");
    Serial.println("Turning on SMARTCONFIG...");
    delay(1000);
    if (wifi.SMARTCONFIG()){
      Serial.println("SMART-ON!");
      Serial.println("Waiting for esptouch...");
      while (!wifi.getWifiInfo(ssid,password))
      {
        //do wait...
      }
      Serial.println("GET WIFI DATA!");
      Serial.print("ssid : ");
      Serial.println(ssid);
      Serial.print("pass : ");
      Serial.println(password);
      while (!wifi.findSMARTEND())
      {
        // do wait...
    
      }
      Serial.println("Ready to go!");
      Serial.println("Please change the controller switch mode & restart the device!");
      return true;
    } else{
      Serial.println("SMART FAILED!");
      return false;
    }
}

bool ESPconnect(void)
{
  if (!wifi.kick()){
    Serial.println("Can not detect Wifi Device!");
    return false;
  }
  if (con_attempt > 40){
    con_attempt=0;
    wifi.restart();
    self_reset();
    delay(3000);
  }
  con_attempt++;
  if (wifi.ConnectAPCheck()){
    con_attempt=0;
    Serial.println("Connected!");
    return true;
  }
  Serial.print("Reconnecting...(");
  Serial.print(con_attempt);
  Serial.println(")");
  EEPROM.get(eeAddress, c_json);
  Serial.println(c_json);
  delay(10000);
  return false;
}

bool ConnecttoServer(void)
{
  // Note : Has connected to AP or GSM Network
  if (wifi.createTCP(ServerName, ServerPort))
  {
    Serial.print("create tcp...");
    Serial.println("ok");
    return true;
  } else {
    Serial.println("create tcp...err");
    return false;
  }
}

bool auth_via_wifi(void)
{
 if (!ESPconnect())
 {
  //Serial.println("--> tidak masuk ESPconnect");
  return false;
 }
 if (!ConnecttoServer())
 {
  //Serial.println("--> tidak masuk ConnecttoServer");
  return false;
 }
 recvFromServer(buffer);
 if (!parse_n_check("cmd", "auth"))
 {
  //Serial.println("--> tidak masuk recvFromServer");
  return false;
 }

 char* hmac_esp8266 = get_hmac();

 StaticJsonBuffer<256> jsonBuffer;
 JsonObject& data = jsonBuffer.createObject();
 data["cmd"] = "auth";
 data["mode"] = "opr";
 data["uid"] = wifi.getSTAMAC();
 data["hmac"] = hmac_esp8266;
 data["via"] = "wifi";

 return send_message(data);
}

void recv_cmd (void)
{
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)buffer);

  if (!root.success())
  {
    return;
  }
  Serial.println("Command Detected!");
  
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
      turn_off_all_driver();
      t.stop(addtimerEvent);
      timeout_counter = 0;
      Serial.println("TCP Closed!");
      wifi.restart();
      delay(3000);
    }
    commandDetected = true;
  }
}

bool auth_reply(uint8_t buffer[256])
{
  //Auth_reply detected
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)buffer);
  
  if (!root.success())  
  {
    Serial.println("Cannot read auth!");
    return false;
  }
  if (!root.containsKey("current_driver"))
  {
    Serial.println("login failed!");
    wifi.releaseTCP();
    init_is_done = false; 
    return false;
  }

  if (!parsing_auth_reply(root))
  {
    Serial.println("parse failed!");
    return false;
  }

  if(!send_current_init(root))
  {
    return false;
  }

  recvFromServer(buffer);
  if(!parse_n_check("status","success"))
  {
    Serial.println("Please check your Internet or Wifi connection!");
    return false;
  }
  addtimerEvent = t.every(1000, adding_timer);
  init_is_done = true;
  Serial.println("Server Initialization done.");
  return true;
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
  bool command_status = false;

  execute_control(root["shift"], root["bit"], root["on"]);

  for (int i = 0; i < 30; i++)
  {
    write_shift_regs(current_driver);
    Serial.println("Write Driver state done.");
    if (!notify_cmd_success (cmd, cmd_type, cmd_id))
    {
      return false;
    }

    recvFromServer(buffer);
    if(!parse_n_check("status","success"))
    {
      Serial.println("command_reply failed!");
      command_status = false;
    } else {
      command_status = true;
      break;
    }
  }
  init_is_done = command_status;
  return command_status;
}

bool keep_alive(void)
{
  //ping detected
  bool keep_alive_status = false;
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["cmd"] = "pong";

  for (int i = 0; i < 30; i++)
  {
    if(!send_message(data))
    {
      return false;
    }
    recvFromServer(buffer);
    if(!parse_n_check("cmd","peng"))
    {
      Serial.println("keep_alive failed!");
      keep_alive_status = false;    
    } else {
      keep_alive_status = true;
      break;
    }
    delay(1000);
  }
  init_is_done = keep_alive_status;
  return keep_alive_status;
}

bool update_panel_status(int n_shiftreg)
{
  //current_driver extracted
  bool update_status = false;
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["cmd"] = "current_update";
  JsonArray& current_panel = data.createNestedArray("current_panel");
  for (int i=0; i<n_shiftreg; i++)
  {
    current_panel.add(SWITCH_ARRAYS[i]);
    //Serial.println(SWITCH_ARRAYS[i]);
  }

  while(init_is_done){
    send_message(data);
    recvFromServer(buffer);
      if(!parse_n_check("status","success"))
      {
        Serial.println(F("Re-update status.."));
        update_status = false;    
      } else {
        update_status = true;
        break;
      }
  }
  return update_status;
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


bool parse_n_check(const char* cmd_class, const char* value)
{
   //uint8_t msg[128];
   uint8_t msg[256];
   for (int i=0; i < sizeof(buffer); i++) {
      msg[i]=buffer[i];
   }
   StaticJsonBuffer<256> jsonBuffer;
   JsonObject& root = jsonBuffer.parseObject((char*)msg);
      
   if (!root.success())
   {
     memset(msg, 0, sizeof(msg));
     //Serial.println("Cannot read!");
     return false;
   }
  
   if (!root.containsKey(cmd_class))
   {
     memset(msg, 0, sizeof(msg));
     //Serial.println("no cmd!");
     return false;
   }

   const char* check = root[cmd_class];
   if ((strcmp (check, value) != 0))
   {
     memset(msg, 0, sizeof(msg));
     //Serial.println("no value auth");
     return false;
   }
   memset(msg, 0, sizeof(msg));
   return true;
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
    for (int i=0; i<n_shiftreg; i++)
    {
      current_driver[i]  = root["current_driver"][i];
    }
    write_shift_regs(current_driver);
    /*for (int j=0; j < root["current_driver"].size(); j++)
    {
      Serial.println(current_driver[j]);
    }*/
    return true;
  }
  return false;
}

bool send_current_init(JsonObject& root)
{
  //current_driver extracted
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["cmd"] = "current_init";
  JsonArray& current_panel = data.createNestedArray("current_panel");
  for (int i=0; i<n_shiftreg; i++)
  {
    current_panel.add(SWITCH_ARRAYS[i]);
    //Serial.println(SWITCH_ARRAYS[i]);
  }
  return send_message(data);
}

bool send_message(JsonObject& data)
{
  String json;
  data.printTo(json);
  json = json + "\r\n";
  const char* charjson;
  charjson = json.c_str();
  Serial.println(charjson);
  if(wifi.send((const uint8_t*)charjson, strlen(charjson)))
  {
   Serial.println("Send OK!");
   return true;
  }
  Serial.println("Send not OK!");
  return false;
}

void recvFromServer (uint8_t buffer1[256])
{
  memset(buffer1, 0, sizeof(buffer1));

  uint8_t buffer[256];
  static uint8_t mux_id = 0;
  uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 10000);
  
    if (len > 0) {
        Serial.print("Received:");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
            buffer1[i] = buffer[i];
        }
    }
    lenceu = len;
}

void write_shift_regs(uint8_t outcoming[NMAX_SHIFTREG])
{
  digitalWrite(latchPin, LOW); //ground latchPin and hold low as long as transmitting
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram
  Serial.print("n_shift di write_shift_regs = ");
  Serial.println(n_shiftreg);
  for (int i=0; i < n_shiftreg; i++)
  {
    //Serial.println("ini : ");
    //Serial.println(outcoming[i]);
    shiftOut(dataPin, clockPin, MSBFIRST, outcoming[(n_shiftreg-1)-i]);
    //all data is shifting...
  }
  digitalWrite(latchPin, HIGH); //return the latchPin to signal the chip and change the Driver state all at once
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram
}

// This code is intended to trigger the shift register to grab values from it's A-H inputs for each shift registers
void read_shift_regs(byte incoming[NMAX_SHIFTREG])
{
  // Trigger loading the state of the A-H data lines into the shift register
  digitalWrite(shld_pin, LOW);
  delayMicroseconds(5); // Requires a delay here according to the datasheet timing diagram
  digitalWrite(shld_pin, HIGH);
  delayMicroseconds(5);

  // Required initial states of these two pins according to the datasheet timing diagram
  pinMode(clk_pin, OUTPUT);
  pinMode(data_pin, INPUT);
  digitalWrite(clk_pin, HIGH);
  digitalWrite(ce_pin, LOW); // Enable the clock

  // Get the A-H values for each shift registers
  for (int i = 0; i < n_shiftreg; i++)
  {
    incoming[i] = shiftIn(data_pin, clk_pin, LSBFIRST);
  }
  digitalWrite(ce_pin, HIGH); // Disable the clock

}

bool compare_switch_state(byte oldstate[NMAX_SHIFTREG])
{
  byte tempstate[NMAX_SHIFTREG];
  bool ChangeOccure = false;
  
  read_shift_regs(tempstate);
  
  for (int j=0; j < n_shiftreg; j++)
  {
    if (tempstate[j] != oldstate[j])
    {
      ChangeOccure = true;
      for (int i=0; i<n_shiftreg; i++)
      {
        oldstate[i] = tempstate[i];
      }
      break;
    }
  }
  return ChangeOccure;
}

void disable_outputs (void){
  digitalWrite(oePin, HIGH);
}

void enable_outputs (void){
  digitalWrite(oePin, LOW);
}

void SERIAL_OUT_SETUP(void)
{
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(oePin, OUTPUT);
  digitalWrite(resetPin, HIGH);
  disable_outputs;
}

void SERIAL_IN_SETUP(void)
{
    // Initialize each digital pin to either output or input
    // We are commanding the shift register with each pin with the exception of the serial
    // data we get back on the data_pin line.
    pinMode(shld_pin, OUTPUT);
    pinMode(ce_pin, OUTPUT);
    pinMode(clk_pin, OUTPUT);
    pinMode(data_pin, INPUT);

    // Required initial states of these two pins according to the datasheet timing diagram
    digitalWrite(clk_pin, HIGH);
    digitalWrite(shld_pin, HIGH);
}

void routine(void)
{
  if (init_is_done){
    if (compare_switch_state(SWITCH_ARRAYS))
    {
      update_panel_status(n_shiftreg);
    }
    Serial.print("counting connection (");
    Serial.print(timeout_counter);
    Serial.println(")");
  }
}

void adding_timer(void)
{
  timeout_counter++;
}

void check_timeout_disconnection(void)
{
  //Serial.println("masuk check_timeout");
  if (timeout_counter > 40)
  {
    Serial.println("Timeout!");
    Serial.println("Disconnecting...");
    t.stop(addtimerEvent);
    timeout_counter = 0;
    turn_off_all_driver();
    init_is_done = false;
  }
}

void turn_off_all_driver(void)
{
  for (int i=0; i < n_shiftreg; i++)
  {
    current_driver[(n_shiftreg-1)-i] = 0x00;
  }
  write_shift_regs(current_driver);
}

void initialize_conn(void)
{
    SERIAL_OUT_SETUP();
    SERIAL_IN_SETUP();

    read_shift_regs(SWITCH_ARRAYS);
    turn_off_all_driver();

    timeout_counter = 0;
    wifi.releaseTCP();
    if(auth_via_wifi())
    {
      enable_outputs();
      recvFromServer(buffer);
      auth_reply(buffer);
    }
}

void whileSMARTCONFIG(void)
{
    STATEA = digitalRead(inPin1);   // read the input pin
    if (STATEA == 0)
    {
          startSMARTCONFIG();
    }

    while (STATEA == 0)
    {
      STATEA = digitalRead(inPin1);
      //Loop until selector change state
    }
}

char* get_hmac(void)
{
   String str_unique_id;
   str_unique_id = "HA-opr-wifi-" + wifi.getSTAMAC();
   char* unique_id;
   unique_id = strdup(str_unique_id.c_str());
   unsigned char* hash = MD5::make_hash(unique_id);
   char *hmac = MD5::make_digest(hash, 16);
   free(hash);
   return hmac;
}

void setup_selfreset(void){
  pinMode(selfreset, OUTPUT);
  digitalWrite(selfreset, HIGH);
}

void self_reset(void){
  digitalWrite(selfreset, LOW);
}


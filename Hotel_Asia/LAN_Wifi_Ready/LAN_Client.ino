#include <SPI.h>
#include <UIPEthernet.h>
//#include <doxygen.h>
#include <ArduinoJson.h>
#include <MD5.h>
#include <Timer.h>

#define NMAX_SHIFTREG 25
// Enter a MAC address and IP address for your controller below.
uint8_t mac[6] = {0x18,0xfe,0x34,0xa6,0x54,0x22};

// Enter the IP address of the server you're connecting to:

#define ServerName  "128.199.208.149"
//#define ServerName  "192.168.130.111"
#define ServerPort  (9123)

// Networking logical
bool anymessage = false;
//bool init_is_done = false;

// Initialize the Ethernet client library
EthernetClient client;

// Carrier for Commands
//bool commandDetected = false;
//uint8_t buffer[128];
//uint8_t buffer[256];

// Shift registers settings
//int n_shiftreg=1;
//static uint8_t current_driver[NMAX_SHIFTREG];
//static uint8_t SWITCH_ARRAYS[NMAX_SHIFTREG];

// Connection Status Timeout Variable
//Timer t;
//int timeout_counter;
//int addtimerEvent;
//int con_attempt=0;
//int addconattemptEvent;

/* Driver Write & Read */
//HARDWARE CONNECTIONS
//Pin connected to OE of 74HC595
//int oePin = 2;
//Pin connected to ST_CP of 74HC595
//int latchPin = 4;
//Pin connected to SH_CP of 74HC595
//int clockPin = 5;
////Pin connected to DS of 74HC595 
//int dataPin = 3;
//Pin connected to MR of 74HC595
//int resetPin = 6;

// Connect the following pins between your Arduino and the 74HC165 Breakout Board
//const int data_pin = 10; // Connect Pin 10 to SER_OUT (serial data out)
//const int shld_pin = 7; // Connect Pin 7 to SH/!LD (shift or active low load)
//const int clk_pin = 9; // Connect Pin 9 to CLK (the clock that times the shifting)
//const int ce_pin = 8; // Connect Pin 8 to !CE (clock enable, active low)

/*** FUNCTIONS FOR ETH ***/

void eth_init(void) {
  // start the Ethernet connection DHCP
  Ethernet.begin(mac);

  Serial.print(F("localIP: "));
  Serial.println(Ethernet.localIP());
  Serial.print(F("subnetMask: "));
  Serial.println(Ethernet.subnetMask());
  Serial.print(F("gatewayIP: "));
  Serial.println(Ethernet.gatewayIP());
  Serial.print(F("dnsServerIP: "));
  Serial.println(Ethernet.dnsServerIP());
}

bool eth_connect(void) {
   // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println(F("connecting..."));

  // if you get a connection, report back via serial:
  if (client.connect(ServerName, ServerPort)) {
    Serial.println(F("connected"));
    return true;
  } else {
    // if you didn't get a connection to the server:
    Serial.println(F("connection failed"));
    return false;
  }
}

void eth_recv(void) {
    // if there are incoming bytes available
  // from the server, read them and print them:
  int size;
  while((size = client.available()) > 0)
  {
    uint8_t msg[256];
    size = client.read(msg,size);
    for (int i=0; i < size; i++) {
      //Serial.print((char)msg[i]);
      buffer[i]=msg[i];
    }
    memset(msg, 0, sizeof(msg));
    anymessage = true;
  }
}

bool eth_connected(void) {
  // if the server's disconnected, stop the client then connect again
  if (!client.connected()) {
    Serial.println();
    Serial.println(F("disconnecting."));
    client.stop();
    // do nothing:
    while (!eth_connect());
  }
  return client.connected();
}

bool eth_send_message(JsonObject& data) {
   String json;
   data.printTo(json);
   json = json + "\r\n";
   const char* charjson;
   charjson = json.c_str();
   
   if (client.connected()) {
      client.print(charjson);
      return true;
   }
   return false;
}

bool eth_check_message(void) {
   /* Parse and check */
   anymessage = false;
   Serial.write(buffer, sizeof(buffer));

   if(parse_n_check("cmd", "auth")){
      return eth_send_auth();
   }

   if(parse_n_check("cmd", "auth_reply")){
      eth_parsing_auth_reply();      
      return eth_send_current_init();
   }

   if(parse_n_check("cmd", "ping")){
      timeout_counter = 0;
      return send_pong();
   }
   
   if(parse_n_check("cmd", "command")){
      t.stop(addtimerEvent);
      timeout_counter = 0;
      return eth_command_control();
   }

   memset(buffer, 0, sizeof(buffer));
   return false;
}

/*bool parse_n_check(const char* cmd_class, const char* value)
{
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
}*/

String mac2str (uint8_t mac[6])
{
  String tmp = String(mac[0],HEX);
  for (int i=1; i < 6; i++) {
    tmp = tmp + ":" + String(mac[i],HEX);
  }
  return tmp;
}

char* eth_get_hmac(void)
{
   String str_unique_id;
   str_unique_id = "HA-opr-eth-" + mac2str(mac);
   char* unique_id;
   unique_id = strdup(str_unique_id.c_str());
   unsigned char* hash = MD5::make_hash(unique_id);
   char *hmac = MD5::make_digest(hash, 16);
   free(hash);
   return hmac;
}

/********************** ALL REPLIES FROM CONTROLLER **********************/

bool eth_send_auth (void) {
   memset(buffer, 0, sizeof(buffer));
   
   // define the reply here
   char* hmac_eth = eth_get_hmac();
   StaticJsonBuffer<256> jsonBuffer2;
   JsonObject& data = jsonBuffer2.createObject();
   data["cmd"] = "auth";
   data["mode"] = "opr";
   data["uid"] = mac2str(mac);
   data["hmac"] = hmac_eth;
   data["via"] = "eth";
   ///////////////////////////////////////////////
   Serial.println(F("auth sent!"));

   return eth_send_message(data);
}

bool eth_send_current_init (void) {
   memset(buffer, 0, sizeof(buffer));

   // define the reply here
   StaticJsonBuffer<256> jsonBuffer;
   JsonObject& data = jsonBuffer.createObject();
   
   data["cmd"] = "current_init";
   JsonArray& current_panel = data.createNestedArray("current_panel");
   for (int i=0; i<n_shiftreg; i++)
   {
     current_panel.add(SWITCH_ARRAYS[i]);
   }
   ///////////////////////////////////////////////
   
   init_is_done = false;
   if(eth_send_message(data)){
     Serial.println(F("current_init sent!"));
     addtimerEvent = t.every(1000, adding_timer);
     init_is_done = true;
     Serial.println(F("Server Initialization done"));
   }

   return init_is_done;
}

bool eth_notify_cmd_success (const char* cmd, const char* type, const char* id) {
   memset(buffer, 0, sizeof(buffer));

   // define the reply here
   StaticJsonBuffer<256> jsonBuffer;
   JsonObject& data = jsonBuffer.createObject();
   data["cmd"]     = cmd;
   data["type"]    = type;
   data["id"]      = id;
   data["status"]  = "success";
   ///////////////////////////////////////////////
   Serial.println("notification sent!");

   return eth_send_message(data);
}

bool send_current_panel (void) {
   memset(buffer, 0, sizeof(buffer));

   // define the reply here
   StaticJsonBuffer<256> jsonBuffer;
   JsonObject& data = jsonBuffer.createObject();
   data["cmd"] = "current_update";
   JsonArray& current_panel = data.createNestedArray("current_panel");
   for (int i=0; i<n_shiftreg; i++)
   {
     current_panel.add(SWITCH_ARRAYS[i]);
   }
   ///////////////////////////////////////////////
   Serial.println(F("current_panel sent!"));

   return eth_send_message(data);
}

bool send_pong (void) {
   memset(buffer, 0, sizeof(buffer));

   // define the reply here
   bool keep_alive_status = false;
   StaticJsonBuffer<256> jsonBuffer;
   JsonObject& data = jsonBuffer.createObject();
   data["cmd"] = "pong";
   ///////////////////////////////////////////////
   Serial.println(F("pong sent!"));

   if(!eth_send_message(data)){
        return false;
   }
   for (int i = 0; i < 30; i++){
      eth_recv();
      if(!parse_n_check("cmd", "peng")){
        //Serial.println(F("keep_alive failed!"));
        keep_alive_status = false;
      } else {
        keep_alive_status = true;
        break;
      }
      //delay(1000);
   }
   
   return keep_alive_status;
}

/**********************************************************************/

/*************************** AUTHENTICATION ***************************/
bool eth_parsing_auth_reply(void) {  
  Serial.println(F("command detected!"));
  uint8_t msg[256];
  for (int i=0; i < sizeof(buffer); i++) {
     msg[i]=buffer[i];
  }
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)msg);

  if (!root.containsKey("current_driver")){
    Serial.println(F("login failed"));
    client.stop();
    init_is_done = false;
    return false;
  }

  if(root["current_driver"].size() != 0){
    n_shiftreg = root["current_driver"].size();
    for (int i=0; i<n_shiftreg; i++)
    {
      current_driver[i]  = root["current_driver"][i];
    }
    write_shift_regs(current_driver);
    return true;
  }
  return false;

}
/**********************************************************************/

/************************** CONTROL FUNCTIONS *************************/
bool eth_command_control(void)
{
  //command detected
  Serial.println("command detected!");
  uint8_t msg[256];
  for (int i=0; i < sizeof(buffer); i++) {
     msg[i]=buffer[i];
  }
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)msg);
  
  const char* cmd = root["cmd"];
  const char* cmd_type = root["type"];
  const char* cmd_id = root["id"];
  int cmd_shift = root["shift"];
  int cmd_bit = root["bit"];
  int cmd_on = root["on"];
  
  bool command_status = false;

  execute_control(root["shift"], root["bit"], root["on"]);
  Serial.println("Write Driver state done.");
  if (!eth_notify_cmd_success (cmd, cmd_type, cmd_id))
  {
     return false;
  }

  /*//differentiate here
  for (int i = 0; i < 30; i++)
  {
    eth_recv();
    if(!parse_n_check("status","success"))
    {
      //Serial.println("command_reply failed!");
      command_status = false;
    } else {
      command_status = true;
      addtimerEvent = t.every(1000, adding_timer);
      break;
    }
  }*/
  write_shift_regs(current_driver);
  //init_is_done = command_status;
  //return command_status;
  return true;
}

/**********************************************************************/

/************************** DRIVER FUNCTIONS **************************/
/*void SERIAL_OUT_SETUP(void)
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

void execute_control(int shift, int shift_bit, int value)
{
  bitWrite(current_driver[shift], shift_bit, value);
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

void turn_off_all_driver(void)
{
  for (int i=0; i < n_shiftreg; i++)
  {
    current_driver[(n_shiftreg-1)-i] = 0x00;
  }
  write_shift_regs(current_driver);
}*/

bool eth_update_panel_status(int n_shiftreg)
{
  //current_driver extracted
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["cmd"] = "current_update";
  JsonArray& current_panel = data.createNestedArray("current_panel");
  for (int i=0; i<n_shiftreg; i++)
  {
    current_panel.add(SWITCH_ARRAYS[i]);
    //Serial.println(SWITCH_ARRAYS[i]);
  }
  return eth_send_message(data);
}

/**********************************************************************/

/************************* ROUTINE FUNCTIONS *************************/
void eth_routine(void)
{
  if (eth_connected()){
    if (compare_switch_state(SWITCH_ARRAYS))
    {
      eth_update_panel_status(n_shiftreg);
    }
    Serial.print("counting connection (");
    Serial.print(timeout_counter);
    Serial.println(")");
  }
}

/*void adding_timer(void)
{
  timeout_counter++;
}*/

/*void check_timeout_disconnection(void)
{
  //Serial.println("masuk check_timeout");
  if (timeout_counter > 45)
  {
    Serial.println("Timeout!");
    Serial.println("Disconnecting...");
    t.stop(addtimerEvent);
    timeout_counter = 0;
    disable_outputs;
    init_is_done = false;
  }
}*/

/**********************************************************************/

/********************* INITIALIZATION FUNCTIONS ***********************/
void eth_initialize_conn(void)
{
    SERIAL_OUT_SETUP();
    SERIAL_IN_SETUP();

    read_shift_regs(SWITCH_ARRAYS);
    turn_off_all_driver();

    if(eth_connect()){
      enable_outputs;
      init_is_done = true;
    }
}

/**********************************************************************/

/*** MAIN FOR ETH ***/
void eth_setup(void){
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  eth_init();
  t.every(5000, eth_routine);
  eth_initialize_conn();
}

void eth_loop(void){
  check_timeout_disconnection();
  if(!init_is_done){
    eth_initialize_conn();
  } else {
    digitalWrite(DRV_ON, LOW);
  }
  eth_recv();
  // as long as there are bytes in the serial queue,
  // read them and send them out the socket if it's open:
  if(anymessage){
    Serial.print(F("-->"));
    eth_check_message();
  }
  eth_connected();
  t.update();
}



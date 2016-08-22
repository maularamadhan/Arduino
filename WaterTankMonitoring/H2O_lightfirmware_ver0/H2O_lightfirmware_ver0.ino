#include <UIPEthernet.h>
#include <ArduinoJson.h>
//#include <MD5.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
uint8_t mac[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// use the numeric IP instead of the name for the server:
#define ServerName  "192.168.130.111"
#define ServerPort  (8080)
bool network_run = true;
int network_attmpt = 0;

// Initialize the Ethernet client library
EthernetClient client;

// Sensor Constant
const uint8_t LVMAX PROGMEM = 740;
const uint8_t LVMIN PROGMEM = 170;
int const analogPin PROGMEM = 5;

// Shift-registers Offline-Level Meter
//Pin connected to ST_CP of 74HC595
const int latchPin PROGMEM = 5;
//Pin connected to SH_CP of 74HC595
const int clockPin PROGMEM = 4;
////Pin connected to DS of 74HC595 
const int dataPin PROGMEM = 6;

// auto-reset
const int selfreset PROGMEM = 3;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  SERIAL_OUT_SETUP();
  eth_init();
}

void loop() {
  if(network_run){
    delay(6000);
    network_attmpt = 0;
  } else {
    delay(1000);
    network_attmpt++;
    if(network_attmpt > 5){
      soft_autoreset();
    }
  }
  /*delay(1000);
  Serial.print("Sensor read: ");
  Serial.println(analogRead(analogPin));
  Serial.print("Level Percent: ");
  Serial.println(read_sensor());*/
  update_level_meter(read_sensor());
  eth_web_post();
}

// Ethernet & WebClient
void eth_init(void){
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // try to congifure using IP address instead of DHCP:
    //Ethernet.begin(mac);
    
  }
  Serial.print(F("localIP: "));
  Serial.println(Ethernet.localIP());
  Serial.print(F("subnetMask: "));
  Serial.println(Ethernet.subnetMask());
  Serial.print(F("gatewayIP: "));
  Serial.println(Ethernet.gatewayIP());
  Serial.print(F("dnsServerIP: "));
  Serial.println(Ethernet.dnsServerIP());
  client.stop();
}

void eth_recv(void) {
    // if there are incoming bytes available
  // from the server, read them and print them:
  int size;
  while((size = client.available()) > 0)
  {
    uint8_t msg[250];
    //uint8_t submessage[125];
    size = client.read(msg,size);
    /*for (int i=0; i < size; i++) {
      Serial.print((char)msg[i]);
    }*/

    if(check_server_response(msg)){
      Serial.println(F("200 OK!"));
      client.stop();
    }
    memset(msg, 0, sizeof(msg));
  }
}

bool eth_connect(void) {
   // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println(F("connecting..."));

  String sh = postval();
  // if you get a connection, report back via serial:
  int count = millis() + 5000;
  if ((client.connect(ServerName, ServerPort))||(millis()==count)) {
    Serial.println(F("connected"));
    client.print(F("POST /es/test HTTP/1.1\r\n"
                   "Host: 192.168.130.111:8080\r\n"
                   "Content-Length: "));
    client.print(sh.length()); //--> size of the message
    client.println();
    client.print(F("H2O-Uid: "));
    client.print(mac2str(mac)); //--> mac as Uid
    client.println();
    client.print(F("H2O_HMAC: "));
    client.print(get_hmac());
    client.print(F("\r\n\r\n")); //--> header-body separator
    client.print(sh); //--> body
    return true;
  } else {
    // if you didn't get a connection to the server:
    Serial.println(F("connection failed"));
    return false;
  }
}

void eth_web_post(void){
  network_run = eth_connect();
  bool msg_rcv = false;
  int count = millis() + 5000;
  while (!msg_rcv){
    eth_recv();
    // if the server's disconnected, stop the client:
    if (!client.connected()) {
      Serial.println();
      Serial.println(F("disconnecting."));
      client.stop();
      msg_rcv = true;
    }
    if (millis() == count){
      break;
    }
  }
}

bool check_server_response(uint8_t msg[250]){
  String s_msg = (char*)msg;
  if(s_msg.indexOf("200 OK") != -1){
    return true;
  }
  return false;
}

/*** Sensor Area ***/
int read_sensor(void){
  int val;
  int percentage;
  val = analogRead(analogPin);
  percentage = map(val, LVMIN, LVMAX, 0, 105);
  if(percentage > 1000){
    percentage = 1000;
  }
  return (percentage/10);
}

void SERIAL_OUT_SETUP(void)
{
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

void update_level_meter(int percentage){
  byte leds = 0x00;
  
  if ((percentage > 4)&&(percentage < 13)){
    leds = B00000001;
  }
  if ((percentage >= 13)&&(percentage < 25)){
    leds = B00000011;
  }
  if ((percentage >= 25)&&(percentage < 38)){
    leds = B00000111;
  }
  if ((percentage >= 38)&&(percentage < 50)){
    leds = B00001111;
  }
  if ((percentage >= 50)&&(percentage < 63)){
    leds = B00011111;
  }
  if ((percentage >= 63)&&(percentage < 75)){
    leds = B00111111;
  }
  if ((percentage >= 75)&&(percentage < 88)){
    leds = B01111111;
  }
  if ((percentage >= 88)&&(percentage <= 100)){
    leds = B11111111;
  }
  
  digitalWrite(latchPin, LOW); 
  shiftOut(dataPin, clockPin, LSBFIRST, leds);
  digitalWrite(latchPin, HIGH);
  delay(500);
}

int notif(int percentage){
  if(percentage < 16) {
    return 1;
  }
  return 0;
}

/*** Auto-reset ***/
void soft_autoreset (void)
{
  /* Setup : pinMode(selfreset, OUTPUT);
             digitalWrite(selfreset, HIGH);*/
  digitalWrite(selfreset, LOW);
}

String mac2str (uint8_t mac[6])
{
  String tmp = "";
  for (int i=0; i < 6; i++) {
    tmp = tmp + "0" + String(mac[i]);
  }
  return tmp;
}

String postval(void){
  StaticJsonBuffer<128> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();
  data["level"]    = read_sensor();
  data["notif"]    = notif(read_sensor());
  
  String json;
  data.printTo(json);
  json = json + "\r\n";
  return json;
}

String get_hmac(void){
  uint8_t codex = 0x85;
  uint8_t temp[6];
  for (int i=0; i<6; i++){
    temp[i] = mac[i]^codex;
  }
  return mac2str(temp);
}

/*** Submessage parsing ***/
//memset(submessage, 0, sizeof(submessage));
//message_body(submessage, msg);
//for (int j=0; j < sizeof(submessage); j++){
//  Serial.print((char)submessage[j]);
//}

/*StaticJsonBuffer<256> jsonBuffer;
JsonObject& root = jsonBuffer.parseObject((char*)submessage);
if (!root.success())
{
  Serial.println(F("Cannot read!"));
}*/

/*void message_body (uint8_t submessage[125], uint8_t msg[250]){
  int open_bracket;
  int close_bracket;
  for(int i=0; i<250; i++){
    if(msg[i] == 123){ // 123 = "{"
      open_bracket=i;
    }
    if(msg[i] == 125){ // 125 = "}"
      close_bracket=i;
      break;
    }
  }
  for(int i=open_bracket; i<=close_bracket; i++){
    submessage[i-open_bracket]=msg[i];
  }
}*/



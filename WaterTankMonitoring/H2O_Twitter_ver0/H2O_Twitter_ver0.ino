#include <SPI.h>
#include <UIPEthernet.h>
#include <UIPServer.h>
#include <UIPClient.h>
#include <ArduinoJson.h>
#include <Twitter.h>
//#include <MD5.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
const uint8_t mac[6] PROGMEM = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// use the numeric IP instead of the name for the server:
#define ServerName  "192.168.130.111"
#define ServerPort  (8080)
bool network_run = true;
int network_attmpt = 0;
int tweet_time;

// Initialize the Ethernet client library
EthernetClient client;

IPAddress myip(192, 168, 5, 10);
IPAddress mysubnet(255, 255, 255, 0);
IPAddress mygateway(192, 168, 5, 5);
IPAddress mydns(192, 168, 8, 8);

const char* const token PROGMEM = "753435446261055488-8uHaIvhCcjInD28tUQluxSKLg9pV4Mu";
const char* const warning PROGMEM = ", Warning!!!";
const char* const level PROGMEM = "Lvl: ";

// Twitter Token
Twitter twitter(token);

// Sensor Constant
const uint16_t LVMAX PROGMEM = 300;
const uint16_t LVMIN PROGMEM = 160;
const int analogPin PROGMEM = 5;

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
  update_level_meter(read_sensor());
  tweet_them();
}

void loop() {
  if(network_run){
    //delay(6000);
    network_attmpt = 0;
  } else {
    delay(1000);
    network_attmpt++;
    if(network_attmpt > 5){
      soft_autoreset();
    }
  }
  update_level_meter(read_sensor());
  /*if (tweet_time > 40) {
    tweet_them();
    tweet_time = 0;
  }
  tweet_time++;*/
  tweet_them();
}

// Ethernet & WebClient
void eth_init(void){
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
  Serial.println(F("Failed to configure Ethernet using DHCP"));
  // try to congifure using IP address instead of DHCP:
  //Ethernet.begin(mac, myip, mydns, mygateway, mysubnet);
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

/*** Sensor Area ***/
uint16_t read_sensor(void){
  uint16_t val;
  uint16_t percentage;
  val = analogRead(analogPin);
  percentage = map(val, LVMIN, LVMAX, 0, 105);
  if(percentage > 100){
    percentage = 100;
  }
  return (percentage);
}

void SERIAL_OUT_SETUP(void)
{
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

void update_level_meter(uint16_t percentage){
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

bool notif(uint16_t percentage){
  if(percentage < 20) {
    return true;
  }
  return false;
}

/*** Auto-reset ***/
void soft_autoreset (void)
{
  /* Setup : pinMode(selfreset, OUTPUT);
             digitalWrite(selfreset, HIGH);*/
  digitalWrite(selfreset, LOW);
}

char* postval(void){
  String temp = "";
  temp = level + String(read_sensor(),DEC) + "%";

  if(notif(read_sensor())){
    temp = temp + warning;
  }

  char* val = strdup(temp.c_str());
  return val;
}

void tweet_them(void){
  //String post_tmp = String(postval());
  Serial.println(F("connecting ..."));
  if (twitter.post(postval())) {
    // Specify &Serial to output received response to Serial.
    // If no output is required, you can just omit the argument, e.g.
    // int status = twitter.wait();
    int status = twitter.wait(&Serial);
    if (status == 200) {
      Serial.println(F("OK."));
    } else {
      Serial.print(F("failed : code "));
      Serial.println(status);
      /*for (int i = 0; i < 5; i++){
        
      }*/
        
    }
  } else {
    Serial.println(F("connection failed."));
  }
}


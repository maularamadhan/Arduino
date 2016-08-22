// Twitter client sketch for ENC28J60 based Ethernet Shield. Uses 
// arduino-tweet.appspot.com as a OAuth gateway.
// Step by step instructions:
// 
//  1. Get a oauth token:
//     http://arduino-tweet.appspot.com/oauth/twitter/login
//  2. Put the token value in the TOKEN define below
//  3. Run the sketch!
//
//  WARNING: Don't send more than 1 tweet per minute!
//  WARNING: This example uses insecure HTTP and not HTTPS.
//  The API key will be sent over the wire in plain text.
//  NOTE: Twitter rejects tweets with identical content as dupes (returns 403)

#include <EtherCard.h>

// OAUTH key from http://arduino-tweet.appspot.com/
const char* const TOKEN PROGMEM = "753435446261055488-8uHaIvhCcjInD28tUQluxSKLg9pV4Mu";
const char* const warning PROGMEM = ", Warning!!!";
const char* const level PROGMEM = "Lvl: ";

// Sensor Constant
const uint16_t LVMAX PROGMEM = 310;
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

// ethernet interface mac address, must be unique on the LAN
const uint8_t mymac[6] PROGMEM = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const char website[] PROGMEM = "arduino-tweet.appspot.com";
bool network_run = true;
int network_attmpt = 0;
int tweet_time;
int tweet_counts = 0;
bool is_tweet_sent = false;

static byte session;

byte Ethernet::buffer[400];
Stash stash;

// ethernet interface ip address
static byte myip[] = { 192,168,10,165 };
// gateway ip address
static byte mygwip[] = { 192,168,8,5 };
// ethernet interface ip address
static byte mydnsip[] = { 192,168,8,8 };
// gateway ip address
static byte mymaskip[] = { 255,255,252,0 };

char* postval(void){
  String temp = "";
  temp = level + String(read_sensor(),DEC) + "%";

  if(notif(read_sensor())){
    temp = temp + warning;
  }

  char* val = strdup(temp.c_str());
  return val;
}

static void sendToTwitter () {
  Serial.println(F("Sending tweet..."));
  byte sd = stash.create();

  //const char tweet[] = "works like magic trick!";
  stash.print("token=");
  stash.print(TOKEN);
  stash.print("&status=");
  stash.print(postval());
  if ((!is_tweet_sent) && (tweet_counts <= 4)){
    stash.print(".");
    tweet_counts++;
  } else if (tweet_counts >= 4){
    tweet_counts = 0;
  }
  stash.println("");
  stash.save();
  int stash_size = stash.size();

  // Compose the http POST request, taking the headers below and appending
  // previously created stash in the sd holder.
  Stash::prepare(PSTR("POST http://$F/update HTTP/1.0" "\r\n"
    "Host: $F" "\r\n"
    "Content-Length: $D" "\r\n"
    "\r\n"
    "$H"),
  website, website, stash_size, sd);

  // send the packet - this also releases all stash buffers once done
  // Save the session ID so we can watch for it in the main loop.
  session = ether.tcpSend();
}

bool check_server_response(const char* msg){
  String s_msg = msg;
  if(s_msg.indexOf("200 OK") != -1){
    return true;
  }
  return false;
}

void setup () {
  Serial.begin(115200);
  Serial.println(F("\n[H2O-HW Twitter]"));
  SERIAL_OUT_SETUP();
  eth_init();
  update_level_meter(read_sensor());
  TweetNReply();
}

void loop () {
  if(network_run){
    delay(6000);
    network_attmpt = 0;
  } else {
    delay(1000);
    network_attmpt++;
    if(network_attmpt > 5){
      //soft_autoreset();
    }
  }
  update_level_meter(read_sensor());
  if (tweet_time > 40) {
    TweetNReply();
    tweet_time = 0;
  }
  tweet_time++;
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

/***   Ethernet Area    ***/
void eth_init(void){
  if (ether.begin(sizeof Ethernet::buffer, mymac, 53) == 0) 
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.staticSetup(myip, mygwip, mydnsip, mymaskip))
    Serial.println(F("static setup failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);
  ether.printIp("mask: ", ether.netmask);
}

bool eth_reply(void){
  ether.packetLoop(ether.packetReceive());

  const char* reply = ether.tcpReply(session);
  if (reply != 0) {
    Serial.println(F("Got a response!"));
    Serial.println(reply);
    is_tweet_sent = check_server_response(reply);
    return true;
  }
  return false;
}

void eth_dnslookup(void){
   if (!ether.dnsLookup(website))
    Serial.println(F("DNS failed"));

   ether.printIp("SRV: ", ether.hisip);
}

void TweetNReply(void){
  eth_dnslookup();
  sendToTwitter();
  while(!eth_reply()){
    Serial.println("123");
    delay(1000);
  }
}

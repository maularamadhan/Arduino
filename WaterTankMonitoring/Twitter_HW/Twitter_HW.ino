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
//#include <avr/wdt.h>

// OAUTH key from http://arduino-tweet.appspot.com/
#define TOKEN   "753435446261055488-8uHaIvhCcjInD28tUQluxSKLg9pV4Mu"
const char* const warning PROGMEM = ", Warning!!!";
const char* const level PROGMEM = "Lvl: ";

// Sensor Constant
const uint16_t LVMAX PROGMEM = 310;
const uint16_t LVMIN PROGMEM = 160;
const int analogPin PROGMEM = 8;

// Shift-registers Offline-Level Meter
//Pin connected to ST_CP of 74HC595
const int latchPin PROGMEM = 5;
//Pin connected to SH_CP of 74HC595
const int clockPin PROGMEM = 4;
////Pin connected to DS of 74HC595 
const int dataPin PROGMEM = 6;

// auto-reset
const int selfreset PROGMEM = A5;

// ethernet interface mac address, must be unique on the LAN
byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0xEE };

const char website[] PROGMEM = "arduino-tweet.appspot.com";

static byte session;

byte Ethernet::buffer[600];
Stash stash;

// ethernet interface ip address
static byte myip[] = { 192,168,128,203 };
// gateway ip address
static byte mygwip[] = { 192,168,128,3 };
// ethernet interface ip address
static byte mydnsip[] = { 192,168,128,3 };
// gateway ip address
static byte mymaskip[] = { 255,255,252,0 };

unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 6000;           // interval at which to blink (milliseconds)

void setup () {
  Serial.begin(57600);
  Serial.println("\n[Twitter Client]");
  SERIAL_OUT_SETUP();
  eth_init();
  eth_dnslookup();
  update_level_meter(read_sensor());
  sendToTwitter();
}

void loop () {
  Serial.println(F("Waiting for reply..."));
  unsigned long waiting_time = millis() + 10000;
  while(!eth_reply()){
  //wdt_enable(WDTO_8S);
    if(millis()>= waiting_time) {
      break;
    }
  }
  
  for(int i = 0; i<50; i++){
    Serial.print("counting..(");
    Serial.print(50-i);
    Serial.println(")");
    delay(6000);
    read_sensor();
  }
  
  update_level_meter(read_sensor());
  //eth_init();
  eth_dnslookup();
  sendToTwitter(); 
}

void sendToTwitter () {
  Serial.println(F("Sending tweet..."));
  byte sd = stash.create();

  //const char tweet[] = "@h2ohw works!";
  stash.print("token=");
  stash.print(TOKEN);
  stash.print("&status=");
  stash.print(level);
  stash.print(read_sensor());
  stash.print("%");
  if(notif(read_sensor())){
    stash.print(warning);
  }
  stash.println();
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
  int freeCount = stash.freeCount();
  if (freeCount <= 3) {   Stash::initMap(56); }
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

void eth_dnslookup(void){
   if (!ether.dnsLookup(website))
    Serial.println(F("DNS failed"));

   ether.printIp("SRV: ", ether.hisip);
}

/*char* postval(void){
  String temp = "";
  temp = level + String(read_sensor(),DEC) + "%";

  if(notif(read_sensor())){
    //temp = temp + warning;
  }

  char* val = strdup(temp.c_str());
  return val;
}*/

/*** Sensor Area ***/
uint16_t read_sensor(void){
  uint16_t val;
  uint16_t percentage;
  //val = analogRead(analogPin); // READ FROM THE REAL SENSOR
  val = random(LVMIN, LVMAX); //FOR TESTING
  percentage = map(val, LVMIN, LVMAX, 0, 105);
  if(percentage > 100){
    percentage = 100;
  }
  return (percentage);
}

bool notif(uint16_t percentage){
  if(percentage < 20) {
    return true;
  }
  return false;
}

void SERIAL_OUT_SETUP(void)
{
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(selfreset, OUTPUT);
  digitalWrite(selfreset, HIGH);
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

bool eth_reply(void){
  ether.packetLoop(ether.packetReceive());

  const char* reply = ether.tcpReply(session);
  if (reply != 0) {
    Serial.println(F("Got a response!"));
    Serial.println(reply);
    //is_tweet_sent = check_server_response(reply);
    return true;
  }
  return false;
}

void soft_autoreset (void)
{
  /* Setup : pinMode(selfreset, OUTPUT);
             digitalWrite(selfreset, HIGH);*/
  digitalWrite(selfreset, LOW);
}

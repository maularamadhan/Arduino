// Simple demo for feeding some random data to Pachube.
// 2011-07-08 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

// Handle returning code and reset ethernet module if needed
// 2013-10-22 hneiraf@gmail.com

// Modifing so that it works on my setup for www.thingspeak.com.
// Arduino pro-mini 5V/16MHz, ETH modul on SPI with CS on pin 10.
// Also added a few changes found on various forums. Do not know what the 
// res variable is for, tweaked it so it works faster for my application
// 2015-11-09 dani.lomajhenic@gmail.com

#include <EtherCard.h>

// change these settings to match your own setup
//#define FEED "000"
#define APIKEY "8NUEO9NCDJWH95JP" // put your key here
#define ethCSpin 10 // put your CS/SS pin here.

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
const int selfreset PROGMEM = 8;

// ethernet interface mac address, must be unique on the LAN
byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0xEE };

const char website[] PROGMEM = "api.thingspeak.com";

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

//unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
//const long interval = 6000;           // interval at which to blink (milliseconds)

void setup () {
  Serial.begin(57600);
  Serial.println("\n[Thingspeak Client]");
  SERIAL_OUT_SETUP();
  eth_init();
  eth_dnslookup();
  update_level_meter(read_sensor());
  sendToThingspeak();
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
  
  for(int i = 0; i<10; i++){
    Serial.print("counting..(");
    Serial.print(10-i);
    Serial.println(")");
    delay(2000);
    update_level_meter(read_sensor());
  }
  
  //eth_init();
  eth_dnslookup();
  sendToThingspeak(); 
}

void sendToThingspeak () {
    // generate two fake values as payload - by using a separate stash,
    // we can determine the size of the generated message ahead of time
    // field1=(Field 1 Data)&field2=(Field 2 Data)&field3=(Field 3 Data)&field4=(Field 4 Data)&field5=(Field 5 Data)&field6=(Field 6 Data)&field7=(Field 7 Data)&field8=(Field 8 Data)&lat=(Latitude in Decimal Degrees)&long=(Longitude in Decimal Degrees)&elevation=(Elevation in meters)&status=(140 Character Message)
    byte sd = stash.create();
    stash.print("field1=");
    stash.print(read_sensor());
    //stash.print("&field2=");
    //stash.print(one);
    //stash.print("&field3=");
    //stash.print(msje);
    stash.save();
  int stash_size = stash.size();

  // generate the header with payload - note that the stash size is used,
    // and that a "stash descriptor" is passed in as argument using "$H"
    Stash::prepare(PSTR("POST /update HTTP/1.0" "\r\n"
      "Host: $F" "\r\n"
      "Connection: close" "\r\n"
      "X-THINGSPEAKAPIKEY: $F" "\r\n"
      "Content-Type: application/x-www-form-urlencoded" "\r\n"
      "Content-Length: $D" "\r\n"
      "\r\n"
      "$H"),
    website, PSTR(APIKEY), stash.size(), sd);

  // send the packet - this also releases all stash buffers once done
  // Save the session ID so we can watch for it in the main loop.
  session = ether.tcpSend();
  int freeCount = stash.freeCount();
  if (freeCount <= 3) {   Stash::initMap(56); }
}

/***   Ethernet Area    ***/
void eth_init(void){
  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0) // 10 for Pro Mini, 53 for Mega, 8 for Uno
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
  val = analogRead(analogPin); // READ FROM THE REAL SENSOR
  //val = random(LVMIN, LVMAX); //FOR TESTING
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

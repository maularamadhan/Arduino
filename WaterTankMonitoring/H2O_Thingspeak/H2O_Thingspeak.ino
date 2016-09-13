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

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
const char website[] PROGMEM = "api.thingspeak.com";
byte Ethernet::buffer[600];
uint32_t timer;
Stash stash;
byte session;

// ethernet interface ip address
static byte myip[] = { 192,168,128,199 };
// gateway ip address
static byte mygwip[] = { 192,168,128,3 };
// ethernet DNS ip address
static byte mydnsip[] = { 192,168,128,3 };
// ethernet subnet ip address
static byte mymaskip[] = { 255,255,252,0 };

//timing variable
int res = 100; // was 0

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

void setup () {
  Serial.begin(57600);
  Serial.println("\n[ThingSpeak example]");
  SERIAL_OUT_SETUP();
  //Initialize Ethernet
  initialize_ethernet();
}


void loop () { 
  //if correct answer is not received then re-initialize ethernet module
  if (res > 220){
    initialize_ethernet(); 
  }
  
  res = res + 1;
  
  ether.packetLoop(ether.packetReceive());
  
  //200 res = 10 seconds (50ms each res)
  if (res == 200) {
   
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

    //Display data to be sent
    Serial.println(read_sensor());


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
    session = ether.tcpSend(); 

 // added from: http://jeelabs.net/boards/7/topics/2241
 int freeCount = stash.freeCount();
    if (freeCount <= 3) {   Stash::initMap(56); } 
  }
   update_level_meter(read_sensor());
   const char* reply = ether.tcpReply(session);
   
   if (reply != 0) {
     res = 0;
     Serial.println(F(" >>>REPLY recieved...."));
     Serial.println(reply);
   }
   delay(300);
}



void initialize_ethernet(void){  
  for(;;){ // keep trying until you succeed 
    //Reinitialize ethernet module
    //pinMode(5, OUTPUT);  // do notknow what this is for, i ve got something elso on pin5
    //Serial.println("Reseting Ethernet...");
    //digitalWrite(5, LOW);
    //delay(1000);
    //digitalWrite(5, HIGH);
    //delay(500);

    if (ether.begin(sizeof Ethernet::buffer, mymac, ethCSpin) == 0){ 
      Serial.println( "Failed to access Ethernet controller");
      continue;
    }
    
    //if (!ether.dhcpSetup()){
      //Serial.println("DHCP failed");
      if (!ether.staticSetup(myip, mygwip, mydnsip, mymaskip)){
        Serial.println(F("static setup failed"));
        continue;
      }
    //}

    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);  
    ether.printIp("DNS: ", ether.dnsip);
    ether.printIp("mask: ", ether.netmask);

    if (!ether.dnsLookup(website))
      Serial.println("DNS failed");

    ether.printIp("SRV: ", ether.hisip);

    //reset init value
    res = 180;
    break;
  }
}

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
  //pinMode(selfreset, OUTPUT);
  //digitalWrite(selfreset, HIGH);
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
  //delay(500);
}

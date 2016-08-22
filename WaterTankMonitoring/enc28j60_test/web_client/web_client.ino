#include <EtherCard.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x22 };

byte Ethernet::buffer[700];
static uint32_t timer;

const char website2[] PROGMEM = "192.168.130.111:8080";

const byte postval[] PROGMEM = "{\r\n\"ahpaduliteuing\" : \"bae weh\",\r\n\"siapamaumakan\" : \"saya!\"\r\n}";


// called when the client request is complete
static void my_callback (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void setup () {
  Serial.begin(57600);
  Serial.println(F("\n[webClient]"));

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) // setting for Arduino Mega
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

  if (!ether.dnsLookup(website2))
    Serial.println("DNS failed");
    
  ether.printIp("SRV: ", ether.hisip);
}

void loop () {
  ether.packetLoop(ether.packetReceive());
  
  if (millis() > timer) {
    timer = millis() + 5000;
    Serial.println();
    Serial.print("<<< REQ ");
    //ether.browseUrl(PSTR("/foo/"), "bar", website, my_callback);
    ether.httpPost(PSTR("/es/test"), website2, 0, (char*)postval, my_callback);
  }
}

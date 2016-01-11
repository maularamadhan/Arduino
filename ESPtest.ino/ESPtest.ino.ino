#include <doxygen.h>
#include <ESP8266.h>

#define SSID        "Tritronik Mobile"
#define PASSWORD    "Tri12@11"
ESP8266 wifi(Serial1, 115200);

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
      //wait for serial
    }
    Serial1.begin(115200);
    //startSMARTCONFIG();
    ESPconnect();
}
void loop()
{
    while (Serial1.available()) {
        Serial.write(Serial1.read());
    }
    while (Serial.available()) {
        Serial1.write(Serial.read());
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
  Serial1.begin(115200);
  if (wifi.setOprToStation() && wifi.sATCWDHCP(1,1))
  {
    if (wifi.ConnectAPCheck()){
      Serial.println("Still Connected!");
      return true;
    } else {
      return wifi.joinAP(SSID, PASSWORD);
    }
  }
}
    

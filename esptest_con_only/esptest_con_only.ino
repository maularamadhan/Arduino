#include <doxygen.h>
#include <ArduinoJson.h>
#include <MD5.h>
#include <ESP8266.h>
#include <Timer.h>
#include <EEPROM.h>

ESP8266 wifi(Serial1, 115200);
int con_attempt=0;

#define ServerName  "128.199.208.149"
#define ServerPort  (9123)

/**///eeprom Memory set
#define eeAddress 0
#define charSize 100

/**///eeprom data carrier
String ssid;
String password;
char c_json[charSize];

int inPin1 = A3;
int STATEA = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
  while(!ESPconnect())
  {
    //do wait
  }
  Serial.println("I am free");

  while(!ConnecttoServer())
  {
    //do wait
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial1.available()) {
      Serial.write(Serial1.read());
  }
  while (Serial.available()) {
      Serial1.write(Serial.read());
  }
}

bool ESPconnect(void)
{
  con_attempt++;
  if (wifi.ConnectAPCheck()){
    con_attempt=0;
    Serial.println("Connected!");
    return true;
  }
  Serial.print("Reconnecting...(");
  Serial.print(con_attempt);
  Serial.println(")");
  delay(10000);
  //wifi.restart();
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

bool startSMARTCONFIG(void)
{
    Serial.print("Setup Starting...\n");
    Serial.println("Turning on SMARTCONFIG...");
    Serial1.begin(115200);
    delay(1000);
    if (wifi.SMARTCONFIG()){
      Serial.println("SMART-ON!");
      Serial.println("Waiting for esptouch...");
      while (!wifi.getWifiInfo(ssid,password))
      {
        //do wait...
      }
      Serial.println(ssid);
      Serial.println(password);
      while (!wifi.findSMARTEND())
      {
        // do wait...
    
      }
      build_eeprom_mem();
      Serial.println("Ready to go!");
      Serial.println("Please change the controller switch mode & restart the device!");
      return true;
    } else{
      Serial.println("SMART FAILED!");
      return false;
    }
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

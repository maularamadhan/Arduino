#include <ArduinoJson.h>
#include <MD5.h>
#include <ESP8266.h>
#include <Timer.h>
#include <EEPROM.h>
#include <SPI.h>
#include <UIPEthernet.h>

#define NMAX_SHIFTREG 25
int n_shiftreg=10;

Timer t;

uint8_t current_driver[NMAX_SHIFTREG];
uint8_t SWITCH_ARRAYS[NMAX_SHIFTREG];
uint32_t lenceu;

//#define ServerName  "128.199.208.149"
#define ServerName  "192.168.130.111"
#define ServerPort  (9123)

// Self-reset
#define selfreset A5

//eeprom memory carrier
#define eeAddress 0
#define charSize 100

//ssid & password stored in eeprom
char c_json[charSize];

ESP8266 wifi(Serial1, 115200);
//SIM900 SIM900(Serial2, 115200);

// Carrier for Commands
bool commandDetected = false;
bool init_is_done = false;
uint8_t buffer[256];

// Connection Status Timeout Variable
int timeout_counter;
int addtimerEvent;
int con_attempt=0;
int addconattemptEvent;

/* Driver Write & Read */
//HARDWARE CONNECTIONS
//Pin connected to OE of 74HC595
int oePin = 2;
//Pin connected to ST_CP of 74HC595
int latchPin = 4;
//Pin connected to SH_CP of 74HC595
int clockPin = 5;
////Pin connected to DS of 74HC595 
int dataPin = 3;
//Pin connected to MR of 74HC595
int resetPin = 6;

// Connect the following pins between your Arduino and the 74HC165 Breakout Board
const int data_pin = 10; // Connect Pin 10 to SER_OUT (serial data out)
const int shld_pin = 7; // Connect Pin 7 to SH/!LD (shift or active low load)
const int clk_pin = 9; // Connect Pin 9 to CLK (the clock that times the shifting)
const int ce_pin = 8; // Connect Pin 8 to !CE (clock enable, active low)


int DRV_ON = 14;
int inPin1 = A3;
int inPin2 = A4;
int STATEA = 0;
int STATEB = 0;

void setup() {
  pinMode(DRV_ON, OUTPUT);
  digitalWrite(DRV_ON, HIGH);
  STATEB = digitalRead(inPin2);
  if (STATEB == 1){
    eth_setup();
  } else {
    esp_setup();
  }
}

void loop() {
  if (STATEB == 1){
    eth_loop();
  } else {
    esp_loop();
  }
}


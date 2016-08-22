//HARDWARE CONNECTIONS
//Pin connected to ST_CP of 74HC595
const int latchPin PROGMEM = 5;
//Pin connected to SH_CP of 74HC595
const int clockPin PROGMEM = 4;
////Pin connected to DS of 74HC595 
const int dataPin PROGMEM = 6;

void setup() {
  // put your setup code here, to run once:
  SERIAL_OUT_SETUP();
  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //write_shift_regs0(DRIVER_ARRAYS);
  //delay(1000);
  //write_shift_regs1(DRIVER_ARRAYS);
  //running_leds();
  update_level_meter();
  //delay(1000);
}

void SERIAL_OUT_SETUP(void)
{
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

void running_leds(void)
{
  byte leds = 0xFF;
  for (int i=0; i < 9; i++)
  {
    digitalWrite(latchPin, LOW); 
    shiftOut(dataPin, clockPin, LSBFIRST, (leds >> (9-i)));
    digitalWrite(latchPin, HIGH);
    delay(500);
  }
}

void update_level_meter(void){
  byte leds = 0x00;
  int percentage;
  percentage = 0;
    
  for (int i=0; i < 9; i++)
  {
    if ((percentage > 1)&&(percentage < 13)){
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
    percentage = percentage + 12;
  }
}


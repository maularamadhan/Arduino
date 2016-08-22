#define NUMBER_OF_SHIFT_REG   2

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

byte DRIVER_ARRAYS[NUMBER_OF_SHIFT_REG];
int x;
int y;

void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);
  x=0; //bit in each shift
  y=0; //n-shift
  Serial.begin(115200);
}

void loop() {
  //count up routine
  if (x == 8)
  {
    x = 0;
    y++;
  }
  if (y == NUMBER_OF_SHIFT_REG)
  {
    y = 0;
  }

  write_shift_regs0(DRIVER_ARRAYS);
  delay(1000);
  write_shift_regs1(DRIVER_ARRAYS);
  delay(1000);
  //running_relay(DRIVER_ARRAYS); // Write the shift registers and change Drivers State
  //delay(500);
  x++;
} 

void write_shift_regs0(byte outcoming[NUMBER_OF_SHIFT_REG])
{
  digitalWrite(latchPin, LOW); //ground latchPin and hold low as long as transmitting
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram
  for (int i=0; i < NUMBER_OF_SHIFT_REG; i++)
  {
    outcoming[(NUMBER_OF_SHIFT_REG-1)-i] = 0x00;
    shiftOut(dataPin, clockPin, MSBFIRST, outcoming[(NUMBER_OF_SHIFT_REG-1)-i]);
    //all data is shifting...
  }
  digitalWrite(latchPin, HIGH); //return the latchPin to signal the chip and change the Driver state all at once
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram
}

void write_shift_regs1(byte outcoming[NUMBER_OF_SHIFT_REG])
{
  digitalWrite(latchPin, LOW); //ground latchPin and hold low as long as transmitting
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram
  for (int i=0; i < NUMBER_OF_SHIFT_REG; i++)
  {
    outcoming[(NUMBER_OF_SHIFT_REG-1)-i] = 0xFF;
    shiftOut(dataPin, clockPin, MSBFIRST, outcoming[(NUMBER_OF_SHIFT_REG-1)-i]);
    //all data is shifting...
  }
  digitalWrite(latchPin, HIGH); //return the latchPin to signal the chip and change the Driver state all at once
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram
}

void running_relay(byte outcoming[NUMBER_OF_SHIFT_REG])
{

  for (int i=0; i < NUMBER_OF_SHIFT_REG; i++)
  {
    outcoming[(NUMBER_OF_SHIFT_REG-1)-i] = 0x00;
  }

  if (x == 0)
  {
    outcoming[y] = 1;
  }
  if (x == 1)
  {
    outcoming[y] = 3;
  }
  if (x == 2)
  {
    outcoming[y] = 7;
  }
  if (x == 3)
  {
    outcoming[y] = 15;
  }
  if (x == 4)
  {
    outcoming[y] = 31;
  }
  if (x == 5)
  {
    outcoming[y] = 63;
  }
  if (x == 6)
  {
    outcoming[y] = 127;
  }
  if (x == 7)
  {
    outcoming[y] = 255;
  }
  
  digitalWrite(latchPin, LOW); //ground latchPin and hold low as long as transmitting
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram

  for (int i=0; i < NUMBER_OF_SHIFT_REG; i++)
  {
    shiftOut(dataPin, clockPin, MSBFIRST, outcoming[(NUMBER_OF_SHIFT_REG-1)-i]);
    //all data is shifting...
    Serial.println(outcoming[(NUMBER_OF_SHIFT_REG-1)-i]);
  }
  //Serial.println(" ");
  Serial.print("ini adalah ");
  Serial.print(y);
  Serial.print(", ");
  Serial.println(x);
    
  digitalWrite(latchPin, HIGH); //return the latchPin to signal the chip and change the Driver state all at once
  delayMicroseconds(5); //Requires delay to make sure things done according to datasheet timing diagram
}

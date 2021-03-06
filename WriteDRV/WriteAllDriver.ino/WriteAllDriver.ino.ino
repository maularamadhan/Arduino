#define NUMBER_OF_SHIFT_REG   5

//HARDWARE CONNECTIONS
//Pin connected to ST_CP of 74HC595
int latchPin = 17;
//Pin connected to SH_CP of 74HC595
int clockPin = 16;
////Pin connected to DS of 74HC595
int dataPin = 14;
//Pin connected to MR of 74HC595
int resetPin = 15;

byte DRIVER_ARRAYS[NUMBER_OF_SHIFT_REG]; // Variable to store Drivers State

// runs once
void setup() {
  SERIAL_OUT_SETUP();
}

void loop() {
  //count up routine
  write_shift_regs0(DRIVER_ARRAYS); // Write the shift registers and change Drivers State
  delay(1000);
  write_shift_regs1(DRIVER_ARRAYS);
  delay(1000);
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

void SERIAL_OUT_SETUP(void)
{
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(500);
  digitalWrite(resetPin, HIGH);
}


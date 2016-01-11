#define NUMBER_OF_SHIFT_REG   5

byte DRIVER_ARRAYS[NUMBER_OF_SHIFT_REG];
byte SHIFT_ARRAYS[NUMBER_OF_SHIFT_REG];

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

void change_driver_state (int GROUP, int NUMBER, int STATE);
{
  bitWrite(DRIVER_ARRAYS[GROUP], NUMBER, STATE);
}

boolean compare_state(byte a[NUMBER_OF_SHIFT_REG], byte b[NUMBER_OF_SHIFT_REG])
{
  for (int i = 0; i < NUMBER_OF_SHIFT_REG; i++)
  {
    if (a[i] != b[i])
    {
      return(false);
    }
    return(true);
  }
}



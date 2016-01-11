/* How many shift registers that daisy chained */
#define NUMBER_OF_SHIFT_REG   5

// HARDWARE CONNECTIONS
// Connect the following pins between your Arduino and the 74HC165 Breakout Board
// Connect pins A-H to 5V or GND or switches or whatever
const int data_pin = 8; // Connect Pin 8 to SER_OUT (serial data out)
const int shld_pin = 5; // Connect Pin 5 to SH/!LD (shift or active low load)
const int clk_pin = 7; // Connect Pin 7 to CLK (the clock that times the shifting)
const int ce_pin = 6; // Connect Pin 6 to !CE (clock enable, active low)

byte SHIFT_ARRAYS[NUMBER_OF_SHIFT_REG]; // Variable to store the values loaded from the shift register

// The part that runs once
void setup() 
{                
  // Initialize serial to gain the power to obtain relevant information, 9600 baud
  Serial.begin(9600);
  // Setup required for the shift-in procedure
  SERIAL_IN_SETUP();
}

// The part that runs to infinity and beyond
void loop() {

  read_shift_regs(SHIFT_ARRAYS); // Read the shift register, it likes that

  // Print out the values being read from the shift register
  Serial.println("\nThe incoming values of the shift register are: ");
  print_all();
  delay(5000); // Wait for some arbitrary amount of time

}

// This code is intended to trigger the shift register to grab values from it's A-H inputs for each shift registers
void read_shift_regs(byte incoming[NUMBER_OF_SHIFT_REG])
{
  // Trigger loading the state of the A-H data lines into the shift register
  digitalWrite(shld_pin, LOW);
  delayMicroseconds(5); // Requires a delay here according to the datasheet timing diagram
  digitalWrite(shld_pin, HIGH);
  delayMicroseconds(5);

  // Required initial states of these two pins according to the datasheet timing diagram
  pinMode(clk_pin, OUTPUT);
  pinMode(data_pin, INPUT);
  digitalWrite(clk_pin, HIGH);
  digitalWrite(ce_pin, LOW); // Enable the clock

  // Get the A-H values for each shift registers
  for (int i = 0; i < NUMBER_OF_SHIFT_REG; i++)
  {
    incoming[i] = shiftIn(data_pin, clk_pin, MSBFIRST);
  }
  digitalWrite(ce_pin, HIGH); // Disable the clock

}

// A function that prints all the 1's and 0's of a byte, so 8 bits +or- 2
void print_byte(byte val)
{
    byte i;
    for(byte i=0; i<=7; i++)
    {
      Serial.print(val >> i & 1, BIN); // Magic bit shift, if you care look up the <<, >>, and & operators
    }
    Serial.print("\n"); // Go to the next line, do not collect $200
}

// A procedure to cek inputed data
void print_all(void)
{
    for (int j = 0; j < NUMBER_OF_SHIFT_REG; j++)
    {
      Serial.print("ABCDEFGH ");
      Serial.print(j);
      Serial.print(": ");
      print_byte(SHIFT_ARRAYS[j]); // Print every 1 and 0 that correlates with A through H for each shift register
    }
}

void SERIAL_IN_SETUP(void)
{
    // Initialize each digital pin to either output or input
    // We are commanding the shift register with each pin with the exception of the serial
    // data we get back on the data_pin line.
    pinMode(shld_pin, OUTPUT);
    pinMode(ce_pin, OUTPUT);
    pinMode(clk_pin, OUTPUT);
    pinMode(data_pin, INPUT);

    // Required initial states of these two pins according to the datasheet timing diagram
    digitalWrite(clk_pin, HIGH);
    digitalWrite(shld_pin, HIGH);
}


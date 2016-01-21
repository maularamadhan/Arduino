int ledPin1 = 14; // LED connected to digital pin 14
int ledPin2 = 8;
int inPin1 = A4;   // pushbutton connected to digital pin 7
int inPin2 = A3;
int val1 = 0;     // variable to store the read value
int val2 = 0;

void setup()
{
  pinMode(ledPin1, OUTPUT);      // sets the digital pin 13 as output
  pinMode(ledPin2, OUTPUT);
  pinMode(inPin1, INPUT);      // sets the digital pin 7 as input
  pinMode(inPin2, INPUT);
}

void loop()
{
  val1 = digitalRead(inPin1);   // read the input pin
  val2 = digitalRead(inPin2);
  digitalWrite(ledPin1, val1);    // sets the LED to the button's value
  digitalWrite(ledPin2, val2);
}

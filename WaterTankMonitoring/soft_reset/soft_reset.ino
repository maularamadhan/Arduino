#define selfreset 3

int count2rest = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(selfreset, INPUT);
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  Serial.println("Start Simulation..");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(count2rest > 25)
  {
    count2rest = 0;
    Serial.println("Stop Simulation");
    pinMode(selfreset, OUTPUT);
    digitalWrite(selfreset, LOW);
    delay(10000);
    pinMode(selfreset, INPUT);
  }
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(600);              // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(200);
  count2rest++;
}

void soft_autoreset (void)
{
  /* Setup : pinMode(selfreset, OUTPUT);
             digitalWrite(selfreset, HIGH);*/
  digitalWrite(selfreset, LOW);
}


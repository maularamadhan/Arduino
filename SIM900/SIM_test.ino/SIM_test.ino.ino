#define SIM900 Serial1
int power_key = 17; // Pin connected to SIM900 powerkey

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial){
    ; // wait for serial port to connect.
  }

  SIM900.begin(9600);               
  SIM900power();  
  delay(20000);  // give time to log on to network.
  /*digitalWrite(power_key, LOW);
  delay(1000);
  digitalWrite(power_key, HIGH);
  delay(2000);*/
  
  // set the data rate for SIM900
  /*SIM900.begin(9600);
  SIM900.println("Hello, world?");*/
}

void loop() {
  callSomeone();
  SIM900power();
  Serial.println("yoho, done!");
  do {} while (1);
  /*// put your main code here, to run repeatedly:
  if (SIM900.available()) {
    Serial.write(SIM900.read());
  }
  if (Serial.available()) {
    SIM900.write(Serial.read());
  }*/
}

void SIM900power()
// software equivalent of pressing the GSM shield "power" button
{
  digitalWrite(power_key, LOW);
  delay(1000);
  digitalWrite(power_key, HIGH);
  delay(5000);
}
 
void callSomeone()
{
  SIM900.println("ATD + +6285352524732;"); // dial US (212) 8675309
  delay(100);
  SIM900.println();
  delay(30000);            // wait for 30 seconds...
  SIM900.println("ATH");   // hang up
}
 

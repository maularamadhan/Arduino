const int LVMAX PROGMEM = 350;
const int LVMIN PROGMEM = 160;

int const analogPin PROGMEM = 5;
uint16_t val = 0;
uint16_t percentage = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:
  val = analogRead(analogPin);
  percentage = map(val, LVMIN, LVMAX, 0, 105);
  if(percentage > 1000){
    percentage = 1000;
  }
  Serial.println(LVMAX-LVMIN);
  Serial.println(val);
  Serial.println(percentage);
}



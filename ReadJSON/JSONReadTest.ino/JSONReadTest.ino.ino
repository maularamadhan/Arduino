String inputString = "";
bool stringComplete = false;

void setup() {
  Serial1.begin(115200);
  Serial.begin(115200);
  inputString.reserve(200);
}

void loop() {
  if(stringComplete) {
    Serial.println(inputString);
    inputString = "";
    stringComplete = false;
  }
}

void serialEvent1() {
  while (Serial1.available()) {
    char inChar = (char) Serial1.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }    
  }
}


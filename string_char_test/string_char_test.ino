String tmp = "mengapa terjadi kepada diriku";
char isi[1024];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("string tmp -> ");Serial.println(tmp);
  Serial.print("char isi (before) -> ");Serial.println(isi);
  strncpy(isi, tmp.c_str(), sizeof(isi));
  isi[sizeof(isi) - 1] = '\0';
  Serial.println(sizeof(isi));
  Serial.print("char isi (after) -> ");Serial.println(isi);
  for (int i=0; i<sizeof(isi); i++)
  {
    Serial.print("ini adalah isi ke-");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(isi[i]);
    if (isi[i] == '\0') {
      break;
    }
  }
  String tmp2(isi);
  Serial.println(tmp2.length());
}

void loop() {
  // put your main code here, to run repeatedly:

}

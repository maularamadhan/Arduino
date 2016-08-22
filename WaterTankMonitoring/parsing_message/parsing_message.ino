#define kurawal "{"
#define t_kurawal "}"
const byte de_buff[600] PROGMEM = "Content-Length: 25\r\nX-Cache: HIT from brahma.tritronik.com\r\nX-Cache-Lookup: HIT from brahma.tritronik.com:3128\r\nVia: 1.0 brahma.tritronik.com:3128 (squid/2.6.STABLE21)\r\nConnection: close\r\n\r\n{\r\n\"status\":\"success\"\r\n}";
int open_bracket;
int close_bracket;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for(int i=0;i<600;i++){
    if(de_buff[i] == *kurawal){
      Serial.print(F("disini kurawal -> "));
      open_bracket=i;
      Serial.println(open_bracket);
    }
    if(de_buff[i] == *t_kurawal){
      Serial.print(F("disini t_kurawal -> "));
      close_bracket=i;
      Serial.println(close_bracket);
    }
  }
  for(int i=open_bracket; i<=close_bracket ; i++){
    Serial.print((char)de_buff[i]);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}



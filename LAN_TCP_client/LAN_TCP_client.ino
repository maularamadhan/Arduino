#include <UIPEthernet.h>

EthernetClient client;
signed long next;

void setup() {

  Serial.begin(9600);

  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
  Ethernet.begin(mac);

  Serial.print("localIP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("subnetMask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("gatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP());

  next = 0;
}

void loop() {

  if (((signed long)(millis() - next)) > 0)
    {
      next = millis() + 5000;
      Serial.println("Client connect");
      // replace hostname with name of machine running tcpserver.pl
//      if (client.connect("server.local",5000))
      if (client.connect(IPAddress(128,199,208,149),9123))
        {
          Serial.println("Client connected");
          client.println("DATA from Client");
          while(client.available()==0)
            {
              if (next - millis() < 0)
                goto close;
            }
          int size;
          while((size = client.available()) > 0)
            {
              uint8_t* msg = (uint8_t*)malloc(size);
              size = client.read(msg,size);
              Serial.print("<<<");
              Serial.write(msg,size);
              Serial.print(">>>");
              for (int i=0; i < size; i++) {
                Serial.print((char)msg[i]);
              }
              free(msg);
            }
close:
          //disconnect client
          Serial.println("Client not disconnect");
          //client.stop();
        }
      else
        Serial.println("Client connect failed");
    }
}



#include <ZsutDhcp.h>
#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <ZsutFeatures.h>
#include <Arduino.h>
#include <string.h>

#define UDP_SERVER_PORT         2138
#define UDP_CLIENT_PORT         2137
#define UDP_CLIENT_IP           192,168,56,103
#define MY_LOCATION 1
#define MY_ID 1

// header values
#define PUBLISH 1
#define SUBSCRIBE 2
#define UNSUBSCRIBE 3
// type values
#define TEMPERATURE 1
#define HUMIDITY 2
#define SOIL_MOISTURE 3
// condition values
#define EQUAL 1
#define LESS_THAN 2
#define GREATER_THAN 3
#define LESS_THAN_OR_EQUAL 4
#define GREATER_THAN_OR_EQUAL 5

#define EMPTY 0
#define DELAY 1000
byte MAC[] = {0x8A, 0x6C, 0x5F, 0x52, 0x18, 0xAA}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a

struct message {
  byte header : 2;
  byte type : 2;
  byte condition : 3;
  byte location : 1;
  byte node_id;
  unsigned int message;
};

unsigned int localPort = UDP_SERVER_PORT;
unsigned int clientPort = UDP_CLIENT_PORT;

uint8_t z0_value;
int hello;
struct message msg;
ZsutIPAddress client_ip;
ZsutEthernetUDP Udp;

void setup() {
  Serial.begin(115200);
  Serial.print(F("Zsut eth udp server init... [")); Serial.print(F(__FILE__));
  Serial.print(F(", ")); Serial.print(F(__DATE__)); Serial.print(F(", ")); Serial.print(F(__TIME__)); Serial.println(F("]"));
  ZsutEthernet.begin(MAC);
  Serial.print(F("My IP address: "));
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(ZsutEthernet.localIP()[thisByte], DEC); Serial.print(F("."));
  }
  Serial.println();
  z0_value = ZsutAnalog0Read();
  hello = 0;
  client_ip = ZsutIPAddress(UDP_CLIENT_IP);
  Udp.begin(localPort);
}

void loop() {
  if (!hello) {
    memset(&msg, EMPTY, sizeof(msg));
    hello = 1;
  }
  static long currentMillis = ZsutMillis();
  if ( ZsutMillis() - currentMillis >= DELAY)  {
    z0_value = ZsutAnalog0Read();
    msg.header = PUBLISH;
    msg.type = HUMIDITY;
    msg.condition = EMPTY;
    msg.location = MY_LOCATION;
    msg.node_id = MY_ID;
    msg.message = z0_value;
    Udp.beginPacket(client_ip, clientPort);
    Udp.write((uint8_t*)&msg, sizeof(msg));
    Udp.endPacket();
    Serial.println(F("Wyslano pakiet do: 192.168.56.103"));
    Serial.println(F("Wiadomosc:"));
    Serial.print(F("Header: ")); Serial.println(msg.header, BIN);
    Serial.print(F("Type: ")); Serial.println(msg.type, BIN);
    Serial.print(F("Condition: ")); Serial.println(msg.condition, BIN);
    Serial.print(F("Location: ")); Serial.println(msg.location, BIN);
    Serial.print(F("Node ID: ")); Serial.println(msg.node_id, BIN);
    Serial.print(F("Message: ")); Serial.println(msg.message, BIN);
    currentMillis = ZsutMillis(); 
  }

}

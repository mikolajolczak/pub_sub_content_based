#include <ZsutDhcp.h>
#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <ZsutFeatures.h>
#include <Arduino.h>
#include <string.h>

#define UDP_SERVER_PORT         2141
#define UDP_CLIENT_PORT         2137
#define UDP_CLIENT_IP           192,168,56,103
#define switchPinLamp              ZSUT_PIN_D1
#define MY_LOCATION 1
#define MY_ID 4
#define BOTTOM 10
#define TOP 25


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

byte MAC[] = {0x8A, 0x6C, 0x5F, 0x52, 0x28, 0xAB}; //MAC adres karty sieciowej, to powinno byc unikatowe - proforma dla ebsim'a

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

int hello;
struct message msg;
unsigned int temp_on;
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
  ZsutPinMode(switchPinLamp, OUTPUT);
  ZsutDigitalWrite(switchPinLamp, HIGH);
  hello = 0;
  client_ip = ZsutIPAddress(UDP_CLIENT_IP);
  Udp.begin(localPort);
}

void loop() {
  if (!hello) {
    msg.header = SUBSCRIBE;
    msg.type = TEMPERATURE;
    msg.condition = LESS_THAN;
    msg.location = MY_LOCATION;
    msg.node_id = MY_ID;
    msg.message = BOTTOM;
    Udp.beginPacket(client_ip, clientPort);
    Udp.write((uint8_t*)&msg, sizeof(msg));
    Udp.endPacket();
    msg.header = SUBSCRIBE;
    msg.type = TEMPERATURE;
    msg.condition = GREATER_THAN;
    msg.location = MY_LOCATION;
    msg.node_id = MY_ID;
    msg.message = TOP;
    Udp.beginPacket(client_ip, clientPort);
    Udp.write((uint8_t*)&msg, sizeof(msg));
    Udp.endPacket();
    ZsutDigitalWrite(switchPinLamp, LOW);
    memset(&msg, 0, sizeof(msg));
    temp_on = 0;
    hello = 1;
  }
  memset(&msg, 0, sizeof(msg));
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    int len = Udp.read((uint8_t*)&msg, sizeof(msg));
    if (len <= 0) {
      Udp.flush();
      return;
    }
  }
  if (msg.type == TEMPERATURE) {
    Serial.println(F("Odebrano pakiet od: 192.168.56.103"));
    Serial.println(F("Wiadomosc:"));
    Serial.print(F("Header: ")); Serial.println(msg.header, BIN);
    Serial.print(F("Type: ")); Serial.println(msg.type, BIN);
    Serial.print(F("Condition: ")); Serial.println(msg.condition, BIN);
    Serial.print(F("Location: ")); Serial.println(msg.location, BIN);
    Serial.print(F("Node ID: ")); Serial.println(msg.node_id, BIN);
    Serial.print(F("Message: ")); Serial.println(msg.message, BIN);
    if (msg.message < 10 && temp_on == 0) {
      temp_on = 1;
      ZsutDigitalWrite(switchPinLamp, HIGH);
      Serial.println(F("Wlaczono lampe"));
      memset(&msg, 0, sizeof(msg));
    }
    if (msg.message > 25 && temp_on == 1) {
      temp_on = 0;
      Serial.println(F("Wylaczono lampe"));
      ZsutDigitalWrite(switchPinLamp, LOW);
      memset(&msg, 0, sizeof(msg));
    }
  }

}

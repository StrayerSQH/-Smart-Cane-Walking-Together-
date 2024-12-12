#include <SoftwareSerial.h>

#define RX 3
#define TX 4

SoftwareSerial BT(RX, TX); 

void setup() {
  Serial.begin(38400);
  BT.begin(38400);
  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);
}

void loop() {
  BT.write('C');
}

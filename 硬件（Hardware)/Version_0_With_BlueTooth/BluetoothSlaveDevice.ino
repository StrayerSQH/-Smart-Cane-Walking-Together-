#include <SoftwareSerial.h>

#define LED_0 A5
#define LED_1 A4
#define LED_2 A3
#define LED_3 A2
#define Light_Sensor 3
#define RX 3
#define TX 4

char val;

SoftwareSerial BT(RX, TX); 

void setup() {
  Serial.begin(38400);
  BT.begin(38400);
  pinMode(LED_0, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(Light_Sensor, INPUT);
}

void loop() {
  if(digitalRead(Light_Sensor) || BT.read() == 'C')
  {
    digitalWrite(LED_0, HIGH);
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, HIGH);
    delay(200); 
  }

  digitalWrite(LED_0, LOW);
  delay(200); 
}


#define LED_0 A5
#define Light_Sensor 3

void setup() {
  Serial.begin(9600);
  pinMode(LED_0, OUTPUT);
  pinMode(Light_Sensor, INPUT);
}

void loop() {
  if(digitalRead(Light_Sensor))
  {
    digitalWrite(LED_0, HIGH);
    delay(500); 
  }
  digitalWrite(LED_0, LOW);
  delay(500); 
}


/*
Andre Arino
Sketch para Learning Shield Arduino
Teste básico das saídas digitais
LEDs apagam em HIGH
LEDs ascendem em LOW
*/

void setup() 
{
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  digitalWrite(10,HIGH);
  digitalWrite(11,HIGH);
  digitalWrite(12,HIGH);
  digitalWrite(13,HIGH);
}

void loop() 
{
  digitalWrite(10,LOW);
  delay(500);
  digitalWrite(10,HIGH);
  digitalWrite(11,LOW);
  delay(500);
  digitalWrite(11,HIGH);
  digitalWrite(12,LOW);
  delay(500);
  digitalWrite(12,HIGH);
  digitalWrite(13,LOW);
  delay(500);
  digitalWrite(13,HIGH);
}

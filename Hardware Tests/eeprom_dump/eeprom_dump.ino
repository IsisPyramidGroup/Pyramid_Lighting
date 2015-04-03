#include <EEPROM.h>

void setup() {
  // put your setup code here, to run once:
  int i;
  byte val;
  
  Serial.begin(9600);
  
  for (i=0; i < 5; i++) {
    val = EEPROM.read(i);
    Serial.print("EEPROM(");
    Serial.print(i);
    Serial.print(") is ");
    Serial.println(val, HEX);
  }  

}

void loop() {
  // put your main code here, to run repeatedly:

}

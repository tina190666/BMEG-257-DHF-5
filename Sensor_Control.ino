#include <EEPROM.h>
int address = 0;
void setup(){
 
  //1 LED
  pinMode(8, OUTPUT);

 //voltage
  pinMode(A1, INPUT);
 
}

void loop() {
  voltage = analogRead(A5) * 5.0/1023;
  resistance = convertR(voltage);
  temperature = convertT(resistance);
  if(address<995){
     EPPROM.write(address,temperature);
  }else{
     EPPROM.update(address%995, temperature);
  }
  address+=3;
  if(temperature > -60){
    digitalWrite(8, HIGH)
  }else{
    digitalWrite(8, LOW)
  }
  
}

double convertR(volt){
  //equations
}

double convertT(temp){
  //equations
}

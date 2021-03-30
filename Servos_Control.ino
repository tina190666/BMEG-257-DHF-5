#include <Servo.h>
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;
Servo servo6;
Servo servo7;

int vacc_in = 0;
int vacc_out = 0;
int col = 11;
int total = 1001;
int rotation;
int layer = 1;
int row = 13;
int count = 1;

void setup() {

  Serial.begin(9600);

  servo1.attach(3, 500, 2500);
  servo1.write(0);
  servo2.attach(4, 500, 2500);
  servo2.write(0);
  servo3.attach(5, 500, 2500);
  servo3.write(0);
  servo4.attach(6, 500, 2500);
  servo4.write(0);
  servo5.attach(7, 500, 2500);
  servo5.write(0);
  servo6.attach(8, 500, 2500);
  servo6.write(0);
  servo7.attach(9, 500, 2500);
  servo7.write(0);
  
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  

}

void loop() {
  while(Serial.available()){
    int num = 0;
    vacc_in = Serial.parseInt();
    Serial.print("Number of vaccines needed = ");
    Serial.println(vacc_in);
    if(vacc_in >= total){
      vacc_in = total;
    }
    if(vacc_in%col == 0){
     vacc_out = vacc_in;
     rotation = vacc_in/col;
    }else{
     vacc_out = (vacc_in/col+1)*11;
     rotation = vacc_in/col+1;
    }
    Serial.print("Number of vaccines withdrawn = ");
    Serial.println(vacc_out);
    total -= vacc_out;
    Serial.print("Number of vaccines left = ");
    Serial.println(total);
    

    while(rotation > 0){
      if(num == row){
       digitalWrite(10, 1);
       digitalWrite(12,HIGH);
       delay(5000);
       digitalWrite(10,0);
       digitalWrite(12, LOW);
       digitalWrite(11,1);
       digitalWrite(13, HIGH);
       delay(5000);
       digitalWrite(11, 1);
       digitalWrite(13, LOW);
       delay(10);
       num = 1;
      }
      motion(layer);
      num++;
      if(count == row){
          layer++;
          count = 1;
      }else{
        count++;
      }
        rotation--;
    }

  }
}


void motion(int layer){
  if(layer == 1){
    servo1.write(360);
    delay(1000);
    servo1.write(0);
    delay(1000);
  }else if(layer == 2){
    servo2.write(360);
    delay(1000);
    servo2.write(0);
    delay(1000);
  }else if(layer == 3){
    servo3.write(360);
    delay(1000);
    servo3.write(0);
    delay(1000);
  }else if(layer == 4){
    servo4.write(360);
    delay(1000);
    servo4.write(0);
    delay(1000);
  }else if(layer == 5){
    servo5.write(360);
    delay(1000);
    servo5.write(0);
    delay(1000);
  }else if(layer == 6){
    servo6.write(360);
    delay(1000);
    servo6.write(0);
    delay(1000);
  }else{
    servo7.write(360);
    delay(1000);
    servo7.write(0);
    delay(1000);
  }
  
}

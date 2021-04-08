/*
 * Function.cpp
 *
 *  Created on: Apr. 7, 2021
 *      Author: tina
 */



#include <Arduino.h>
#include "Function.h"
#include <math.h>
#include <Servo.h>
#include <SD.h>
#include <SPI.h>

extern "C"{


  // This function takes analogue input from the sensor and returns the value.
  int measure(){
    return analogRead(A0);
  }

  //This function coverts the analogue input from the sensor to the temperature of the vaccines
  float convert(int volt_analog){

    //convert analogue input to voltage
    float volt = 5.0 / 1024.0 * volt_analog;

    //calculate voltage before gain
    float volt_o = (volt * 0.032 / 5.0) + 3.892;

    //convert voltage to resistance
    float resistance = (4500.0 - 1000.0 * volt_o) / (4.5 + volt_o);

    //convert resistance to temperature
    float temp = (resistance - 100.33) / (2.0 / 5.0);

    return temp;
  }

  //This function checks if the temperature of the vaccines is above the optimal temperature range (-60C)
  bool ThresholdTemp(float temp){
    return (temp >= -60.0);
  }

  //This function checks if the retrieval compartment is opened by the user.
  bool accessState(){
    return (digitalRead(11) == 0);
  }

  //This function turns on the red LED if the temperatire of the vaccines is above the optimal temperature range (-60C), and turns off the led if the temperature is within the optimal temperature range.
  void redLED(bool thresh, int pinNumber){
    if(thresh){
      digitalWrite(pinNumber, HIGH);
    }else{
      digitalWrite(pinNumber, LOW);
    }
  }


  //This function tells the user to slide the switch connected to the base of the catch compartment, allowing the vaccines to drop into the retrieval compartment
  void catchSlide(int pinNumber){
     Serial.print("flip the switch");  //requests the user to slide the switch
     delay(5000);                      //wait for 5 seconds to allow the user to slide switch
     while(digitalRead(10)==1){       //when switch is slided
       delay(13000); //wait for 13 seconds, 13 rows in the catcher dropping to retrieval compartment at the rate of 1 row/second
       Serial.print("flip the switch back");  //requests the user to slide the switch back
    }
  }

  //This function blinks the blue LED to alert the user that the retrieval compartment is ready to be opened.
  void blinkBlue(int pinNumber){
    digitalWrite(pinNumber, HIGH);
    delay(500);
    digitalWrite(pinNumber, LOW);
    delay(500);
    digitalWrite(pinNumber, HIGH);
  }

  //rotate the servo corresponding to the lowest non-empty layer.
  void pos(int layer, Adafruit_PWMServoDriver pwm, int col_count){
    if(layer == 1){
        for(uint16_t i=0; i<4096; i+= 8){
            pwm.setPwn(0,0,(i+ (4096/16)*0 % 4096));
        } //rotate the servo 13.85 degrees -- corresponds to dispensing one row of the vaccines
        delay(1000);
      }else if(layer == 2){
        for(uint16_t i=0; i<4096; i+= 8){
            pwm.setPwn(1,0,(i+ (4096/16)*1 % 4096));
        }
        delay(1000);
      }else if(layer == 3){
        for(uint16_t i=0; i<4096; i+= 8){
            pwm.setPwn(2,0,(i+ (4096/16)*2 % 4096));
        }
        delay(1000);
      }else if(layer == 4){
        for(uint16_t i=0; i<4096; i+= 8){
            pwm.setPwn(3,0,(i+ (4096/16)*3 % 4096));
        }
        delay(1000);
      }else if(layer == 5){
        for(uint16_t i=0; i<4096; i+= 8){
            pwm.setPwn(4,0,(i+ (4096/16)*4 % 4096));
        }
        delay(1000);
      }else if(layer == 6){
        for(uint16_t i=0; i<4096; i+= 8){
            pwm.setPwn(5,0,(i+ (4096/16)*5 % 4096));
        }
        delay(1000);
      }else{
        for(uint16_t i=0; i<4096; i+= 8){
            pwm.setPwn(6,0,(i+ (4096/16)*6 % 4096));
        }
        delay(1000);
      }
  }


  //This function logs the temperature of the vaccines (and corresponding time in hours) to SD card.
  void tempLog(long milli, float temperature, File tempFile){
    long hour = milli / (1000.0 * 60.0 * 60.0);  //convert time to hours
    tempFile.println(hour + "     " + temperature);
  }

  //This function logs the number of doses remaining in the shipper (and corresponding time in hours) to the SD card.
  void vaccineLog(int vacc_out, int total, long milli, File vaccFile){
    long hour = milli / (1000.0 * 60.0 * 60.0);  //convert time to hours
    int remain = total - vacc_out;
    vaccFile.println(hour + "     " + remain);
  }

  //This function returns the closest (and higher) number of vaccines (in units of 11 vaccines) that can be withdrawn to the user request. (eg. if user inputs 10, vacc_out = 11 doses; if user inputs 12 doses, vacc_out = 22 doses)
  int vaccine_out(int vacc_in, int total, int col){
    if(vacc_in >= total || vacc_in % col == 0){
      return vacc_in;
    }
    return (vacc_in / col + 1)*11;

  }

  //This function returns the number of rotations to be performed by the servos to for the withdrawal.
  int rot(int vacc_out, int col){
    return vacc_out / col;
  }

  //This function checks if a layer in the shipper is empty (has no vaccines).
  bool layerEmpty(int col_count){
    return (col_count % 13 == 0);
  }

  //This function checks if the catch compartment is full (13 columns in the compartment = 11 * 13 = 143 doses)
  bool catchFull(int rotation){
    return (rotation == 13);
  }

  //This function turns on or off the blue LED depending on the access state of the retrieval compartment. (if compartment is opened, turn on the blue LED, if compartment is closed, turn off the blue LED)
  void blueLED(bool full, int pinNumber){
    if(full){
      digitalWrite(pinNumber, HIGH);
    }else{
      digitalWrite(pinNumber, LOW);
    }
  }



}

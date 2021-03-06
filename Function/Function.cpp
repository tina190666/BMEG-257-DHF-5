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
#include <LiquidCrystal.h>

extern "C"{


  // This function takes analogue input from the sensor and returns the value.
  int measure(){
    return analogRead(A0);
  }

  //This function coverts the analogue input from the sensor to the temperature of the vaccines
  float convert(int volt_analog){

    //convert analogue input to voltage
    //float volt = 5.0 / 1024.0 * volt_analog;

    //calculate voltage before gain
    float volt_o = (volt_analog / 443.74 );

    //convert voltage to resistance
    float resistance = (7832.0 / 101.0 - (7832.0 * volt_o / 9.0)) / (1.0 - (1.0 / 101.0) + volt_o / 9.0);

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
    return (digitalRead(7) == 0);
  }

  //This function turns on the red LED if the temperature of the vaccines is above the optimal temperature range (-60C), and turns off the led if the temperature is within the optimal temperature range.
  void redLED(bool thresh, int pinNumber){
    if(thresh){
      digitalWrite(pinNumber, HIGH);
    }else{
      digitalWrite(pinNumber, LOW);
    }
  }


  //This function tells the user to slide the switch connected to the base of the catch compartment, allowing the vaccines to drop into the retrieval compartment
  void catchSlide(int total, LiquidCrystal lcd){
      lcd.clear();
      lcd.print("Flip Switch");
      delay(5000);                      //wait for 5 seconds to allow the user to slide switch
      delay(13000); //wait for 13 seconds, 13 rows in the catcher dropping to retrieval compartment at the rate of 1 column /second
      lcd.clear();
      lcd.print("Flip Switch Back");
      delay(5000);
      lcd.clear();
      lcd.print("Vaccine Count: ");
      lcd.setCursor(0,1);
      lcd.print(total);
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


  //This function logs the temperature of the vaccines (and corresponding time in minutes) to SD card.
  void tempLog(long milli, float temperature, File tempFile){
    long hour = milli / (1000.0 * 60.0);  //convert time to minutes
    tempFile.println(hour + "     "); 
    tempFile.print(temperature);
  }

  //This function diplays the number of vaccines remaining in the shipper to LCD..
  void vaccineDisplay(int vacc_out, LiquidCrystal lcd){
    lcd.clear();
    lcd.print("Vaccine Count: ");
    lcd.setCursor(0,1);
    lcd.print(vacc_out);
  }
  
  void vaccineDisplayOut(int vacc_out, LiquidCrystal lcd){
    lcd.clear();
    lcd.print("Vaccine Output: ");
    lcd.setCursor(0,1);
    lcd.print(vacc_out);  
  }
  
  void completeDisplay(LiquidCrystal lcd, int total){
    lcd.clear();
    lcd.print("Process Completed");
    delay(2000);
    lcd.clear();
    lcd.print("Vaccine Count: ");
    lcd.setCursor(0,1);
    lcd.print(total);
  }

  //This function returns the closest (and higher) number of vaccines (in units of 11 vaccines) that can be withdrawn to the user request. (eg. if user inputs 10, vacc_out = 11 doses; if user inputs 12 doses, vacc_out = 22 doses)
  int vaccine_out(int vacc_in, int total, int col){
    if(vacc_in >= total || vacc_in % col == 0){
      return vacc_in;
    }
    return (vacc_in / col + 1) * 11;

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

  //This function checks if the current time is 10min away from the last time the temperature was logged
  bool LogTime (long milli, long prevMilli, long period){
    return ( (milli - prevMilli) >= period);
  }
  
  // This function takes analogue input from the knob and returns the value.
  int measureVacc(){
    return analogRead(A1);
  }

  // This function finds the number of vaccines required by the user (from knob input).
  int convertVacc(int vacc_analog, int pos_knob){
     int pos_current = map(vacc_analog, 1, 1024, 1, 255);
     int vacc = map(pos_current - pos_knob, 1, 255, 3, 1009);
     return vacc; 
  }


}

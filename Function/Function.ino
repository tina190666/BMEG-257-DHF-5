#include <Servo.h>
#include "Function.h"
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);


File temp_Log;      //file used to save temperature data of the vaccines in the shipper.

int col = 11;         //number of columns of vaccines in the shipper
int total = 1001;    //total number of doses remaining in the shipper.
int layer = 1;       //lowest non-empty layer in the shipper
int row = 13;        //number of rows of vaccines per layer in the shipper.
int count_col = 0;   //number of columns of vaccines withdrawn from the shipper
long prevMilli = 0;  //the time of the last occurence of logging temperature data to file.
long period = 600000; //10 minutes: 1000*60*10 = 600000
int pos_knob = 1;     //position of the knob controlled by the yser.

void setup() {
  Serial.begin(9600);  //Initialize serial monitor.

  pinMode(6, INPUT);     //Setting pin 6 as the input pin for the sliding switch controlling the base of the catch compartment.
  
  pinMode(7, INPUT_PULLUP);  //Setting pin 7 as the input pin for the pushbutton that represents the state of the retrieval compartment (opened / closed)
       
  pinMode(8, OUTPUT);      //Setting pin 8 as the output pin for the red LED that indicates the thermal state of the vaccines.
                        
  pinMode(9, OUTPUT);     //Setting pin 9 as the output pin for the blue LED that indicates the access state of the retrieval compartment.
  
  pinMode(A0, INPUT);   //Setting Analog 0 pin as the input pin where the sensor opamp of the sensor will be connected to.

  pinMode(A1, INPUT);  //Setting Analog 1 pin as the input pin where the knob controlling the number of vaccines to be withdrawn will be connected to.
  
  lcd.begin(16, 2); //intialize LCD
  lcd.print("Vaccine Count: "); //print Vaccine Count: on the first row of the LCD

  // if you want to really speed stuff up, you can go into 'fast 400khz I2C' mode
// some i2c devices dont like this so much so if you're sharing the bus, watch
// out for this!

pwm.begin();
pwm.setPWMFreq(1600);  // This is the maximum PWM frequency

// save I2C bitrate
uint8_t twbrbackup = TWBR;
// must be changed after calling Wire.begin() (inside pwm.begin())
TWBR = 12; // upgrade to 400KHz!
}

void loop(){
  //Initiates SD card module
  if(!SD.begin(13)){
    Serial.println("Card failed or does not exist");    //message printed when card fails to be detected.
    while(1);
  }
  Serial.println("Card Initialized");     //message printed when SD card is successfully initialized
  temp_Log = SD.open("Temperature.txt", FILE_WRITE);    //open temp_log file to store the temperature of vaccines every 10 minutes.

  if(temp_Log){          //when temp_Log is successfully opened
    int volt_analog = measure();     //reads analogue input that sent to the Arduino from A0 pin
    float temperature = convert(volt_analog);  //converts analogue input to the temperature of the vaccines
    bool tempThresh =  ThresholdTemp(temperature);   //checks if the temperature is above -60 C
    long milli = millis();                         //initiates time variable
    bool logTime = LogTime(milli, prevMilli, period); //checks if the current time is more than or equal to 10 minutes away from the last time temperature data was logged
    if(logTime){                              //if the current time is more than or equal to 10 minutes away from the last time temperature data was logged
      tempLog(milli, temperature, temp_Log);    //log the temperature of the vaccines to temp_Log file and set current time to prevMilli
      prevMilli = milli;
    }
    redLED(tempThresh, 1);                        //turn on red led to alert user of the vaccine's thermal stability being at risk if temperature of the vaccines is greater than -60C
    if(!tempThresh){                    //if vaccines are thermally stable
      int vacc_analog = measureVacc();   //read analogue input sent to the Arduino from A1 pin
      int vacc_in = convertVacc(vacc_analog, pos_knob); //convert analogue input to number of doses.
      if(vacc_in != 0){                  //if the number of doses is not equal to 0, start dispensing process
        pos_knob = map(vacc_analog, 1, 1024, 1, 255); //update position variable of the knob. 
        int vacc_out = vaccine_out(vacc_in, total, col);  //calculate the closest number of vaccines to the input that can be output from the shipper.
        vaccineDisplay(vacc_out, total, lcd);  //display the number of doses left in the shipper after the determined amount is withdrawn on the LCD
        int rotation = (vacc_out, col);          //calculate the number of rotations of the servo required to dispense vaccines

        //Initialize variables to keep track of number of rotations and the number of rows dispensed since the first withdrawal.
        int count_rot = 0;
        int count_catch = 0;

        while(count_rot < rotation){             //when the number of rotations performed is less than the required amount for the withdrawal
          pos(layer, pwm, count_col);  //rotate the servo that controls the lowest non-empty layer in the shipper to dispense a column of vaccine into the catch compartment.
          count_rot++;
          count_catch++;
          count_col++;
          bool layer_emp = layerEmpty (count_col);  //check if the rotated layer is now empty
          if(layer_emp){
            layer++;                              //if the layer is empty, increment layer by 1 (for next round of rotation or withdrawal)
          }

        if(catch_full || count_rot == rotation){   //if the catch compartment is full or if all required doses is in the catch compartment
            boolean state = true;          
            blueLED(state, 9);                   //turn on the blue LED to indicate catch compartment is ready to drop vaccines into the retrieval compartment
            catchSlide(6);                       //slide the base of the catch compartment, allowing the vaccines to drop into the retrieval compartment
            while(!accessState()){      //when the retrieval compartment is not opened by the user
              blinkBlue(11);            //blink the blue LED to alert the user to pick up vaccines from the retreival compartment
            }

            blueLED(accessState(), 9); //blueLED stays on when retrieval compartment is opened.

            while(accessState()){     //while the retreival compartment is opened:
              //measure data from temperature sensor and convert to the temperature of the vaccines
              volt_analog = measure();                    
              temperature = convert(volt_analog);
              tempThresh = ThresholdTemp(temperature);
              milli = millis();
              bool logTime = LogTime(milli, prevMilli, period); //checks if the current time is more than or equal to 10 minutes away from the last time temperature data was logged
              if(logTime){                              //if the current time is more than or equal to 10 minutes away from the last time temperature data was logged
                tempLog(milli, temperature, temp_Log);    //log the temperature of the vaccines to temp_Log file and set current time to prevMilli
                prevMilli = milli;
              }
            }


            while(tempThresh){   //after the retrieval compartment is closed by the user and if the temperature of vaccines is above -60C; looped until the vaccine temperature is within the optimal range.
              //measure data from temperature sensor and convert to the temperature of the vaccines
              volt_analog = measure();
              temperature = convert(volt_analog);
              tempThresh = ThresholdTemp(temperature);
              milli = millis();
              bool logTime = LogTime(milli, prevMilli, period); //checks if the current time is more than or equal to 10 minutes away from the last time temperature data was logged
              if(logTime){                              //if the current time is more than or equal to 10 minutes away from the last time temperature data was logged
                tempLog(milli, temperature, temp_Log);    //log the temperature of the vaccines to temp_Log file and set current time to prevMilli
                prevMilli = milli;
              }
            }
            count_catch = 0;
          }

         }
    }

    //close temp_Log file.
    temp_Log.close();
  }


  if(total == 0){  //when all vaccines have been withdrawn, delay the loop for 1000 seconds to represent the end of the program execution.
    delay(1000000);
  }

}

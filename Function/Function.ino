#include <Servo.h>
#include "Function.h"
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

//declare the 7 servos (1 per conveyor belt)
// Servo servo1;
// Servo servo2;
// Servo servo3;
// Servo servo4;
// Servo servo5;
// Servo servo6;
// Servo servo7;

File vacc_Log;
File temp_Log;

int vacc_in = 0;
int vacc_out = 0;
int col = 11;
int total = 1001;
int layer = 1;
int row = 13;
int count_col = 0;
long prevMilli = 0;
long period = 10800000; //3 hours: 3*60*60*1000

void setup() {
  Serial.begin(9600);  //Initialize serial monitor for user input and instructional output.

  pinMode(10, INPUT);     //Setting pin 10 as the input pin for the sliding switch controlling the base of the catch compartment.

  pinMode(11, INPUT_PULLUP);  //Setting pin 11 as the input pin for the pushbutton that represents the state of the retrieval compartment (opened / closed)

  pinMode(12, OUTPUT);      //Setting pin 12 as the output pin for the red LED that indicates the thermal state of the vaccines.

  pinMode(13, OUTPUT);     //Setting pin 23 as the output pin for the blue LED that indicates the access state of the retrieval compartment.

  pinMode(A0, INPUT);   //Setting Analog 0 pin as the input pin where the sensor opamp of the sensor will be connected to.

 //Servos
 /*
  * for each servo:
  *   attach servo to a pin on Arduino
  *   setting the initial position of the servo to 0 degrees
  */
  // servo1.attach(3, 500, 2500);
  // servo1.write(0);
  // servo2.attach(4, 500, 2500);
  // servo2.write(0);
  // servo3.attach(5, 500, 2500);
  // servo3.write(0);
  // servo4.attach(6, 500, 2500);
  // servo4.write(0);
  // servo5.attach(7, 500, 2500);
  // servo5.write(0);
  // servo6.attach(8, 500, 2500);
  // servo6.write(0);
  // servo7.attach(9, 500, 2500);
  // servo7.write(0);

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
  if(!SD.begin(4)){
    Serial.println("Card failed or does not exist");    //message printed when card fails to be detected.
    while(1);
  }
  Serial.println("Card Initialized");     //message printed when SD card is successfully initialized
  vacc_Log = SD.open("Vaccine_Count.txt", FILE_WRITE);   //open vacc_Log file to store the number of doses available in the shipper.
  temp_Log = SD.open("Temperature.txt", FILE_WRITE);    //open temp_log file to store the temperature of vaccines every 3 hours.

  if(vacc_Log && temp_Log){          //when both files are successfully opened
    int volt_analog = measure();     //reads analogue input that sent to the Arduino from A0 pin
    float temperature = convert(volt_analog);  //converts analogue input to the temperature of the vaccines
    bool tempThresh =  ThresholdTemp(temperature);   //checks if the temperature is above -60 C
    long milli = millis();                         //initiates time variable
    if(milli - prevMilli >= period){              //if the current time is more than or equal to 3 hours away from the last time temperature data was logged
      tempLog(milli, temperature, temp_Log);      //log the temperature of the vaccines to temp_Log file and set current time to prevMilli
      prevMilli = milli;
    }
    redLED(tempThresh, 1);                        //turn on red led to alert user of the vaccine's thermal stability being at risk if temperature of the vaccines is greater than -60C
    if(digitalRead(1) == LOW){                    //if vaccines are thermally stable
      while(Serial.available()){                  //when the user inputs the desired number of doses to be withdrawn
        vacc_in = Serial.parseInt();              //parse the input from String to int
        int vacc_out = vaccine_out(vacc_in, total, col);  //calculate the closest number of vaccines to the input that can be output from the shipper.
        vaccineLog(vacc_out, total, millis(), vacc_Log);  //log the number of doses left in the shipper after the determined amount is withdrawn along with the current time
        int rotation = (vacc_out, col);          //calculate the number of rotations of the servo required to dispense vaccines

        //Initialize variables to keep track of number of rotations and the number of rows dispensed since the first withdrawal.
        int count_rot = 0;
        int count_catch = 0;


        while(count_rot < rotation){             //when the number of rotations performed is less than the required amount for the withdrawal
          pos(layer, pwm, count_col);  //rotate the servo that controlls the lowest non-empty layer in the shipper to dispense a layer of vaccine into the catch compartment.
          count_rot++;
          count_catch++;
          count_col++;
          bool layer_emp = layerEmpty (count_col);  //check if the rotated layer is now empty
          if(layer_emp){
            layer++;                              //if the layer is empty, increment layer by 1 (for next round of rotation or withdrawal)
          }

          bool catch_full = catchFull(count_catch); //check if the catch compartment is full (11 * 13 = 143 doses)

          if(catch_full || count_rot == rotation){   //if the catch compartment is full or if all required doses is in the catch compartment
            boolean state = true;
            blueLED(state, 13);                   //turn on the blue LED to indicate catch compartment is ready to drop vaccines into the retrieval compartment
            catchSlide(10);                       //slide the base of the catch compartment, allowing the vaccines to drop into the retrieval compartment
            while(!accessState()){      //when the retrieval compartment is not opened by the user
              blinkBlue(11);            //blink the blue LED to alert the user to pick up vaccines from the retreival compartment
            }

            blueLED(accessState(), 13); //blueLED stays on when retrieval compartment is opened.

            while(accessState()){     //while the retreival compartment is opened:
              //measure data from temperature sensor and convert to the temperature of the vaccines
              volt_analog = measure();
              temperature = convert(volt_analog);
              tempThresh = ThresholdTemp(temperature);
              milli = millis();
              //log temperature of the vaccines if the current time is 3 hours or longer away from the last time the data was logged
              if(milli - prevMilli >= period){
               tempLog(milli, temperature, temp_Log);
               prevMilli = milli;
              }
            }


            while(digitalRead(1) == HIGH){   //after the retrieval compartment is closed by the user and if the temperature of vaccines is above -60C; looped until the vaccine temperature is within the optimal range.
              //measure data from temperature sensor and convert to the temperature of the vaccines
              volt_analog = measure();
              temperature = convert(volt_analog);
              tempThresh = ThresholdTemp(temperature);
              milli = millis();
              //log temperature of the vaccines if the current time is 3 hours or longer away from the last time the data was logged
              if(milli - prevMilli >= period){
               tempLog(milli, temperature, temp_Log);
               prevMilli = milli;
              }
            }
            count_catch = 0;
          }

        }
      }
    }

    //close vacc_Log and temp_Log files.
    vacc_Log.close();
    temp_Log.close();
  }


  if(total == 0){  //when all vaccines have been withdrawn, delay the loop for 1000 seconds to represent the end of the program execution.
    delay(1000000);
  }

}

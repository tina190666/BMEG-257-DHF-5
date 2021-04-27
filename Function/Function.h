/*
 * Function.h
 *
 *  Created on: Apr. 7, 2021
 *      Author: tina
 */
#include <Servo.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <LiquidCrystal.h>

#ifndef FUNCTION_H
extern "C" {
  #define FUNCTION_H_

  //measure function that performs the analogRead from temperature sensor and returns the value.
  int measure();

  //convert function that takes the analogRead value from the sensor and converts it to temperature.
  float convert(int volt_analog);

  //ThresholdTemp function that checks if the temperature of the vaccines is greater than its optimal temperature range.
  bool ThresholdTemp(float temp);

  //accessState function that checks if the retrieval compartment is opened.
  bool accessState();

  //redLED function that turns on or off the red LED based on the temperature state of the vaccines (on if temperature is outside of optimal temperature range).
  void redLED(bool thresh, int pinNumber);

  //catchSlide function that tells the user to slide the base of the catch compartment, allowing the vaccines to drop from the catch compartment to the retrieval compartment.
  void catchSlide(int total, LiquidCrystal lcd);
  
  //blinkBlue function that blinks the blue LED when retrieval compartment is not opened but ready to to be opened.
  void blinkBlue(int pinNumber);

  //pos function that rotates the conveyor belt of the lowest non-empty layer in the shipper by 13.85 degrees (dispenses one column)
  void pos(int layer, Adafruit_PWMServoDriver pwm, int col_count);

  //tempLog function that logs the temperature (celcius) and time (hours) into file in SD card.
  void tempLog(long milli, float temperature, File tempFile);

  //vaccineDisplay function that diplays the number of vaccines remaining in the shipper to LCD.
  void vaccineDisplay(int vacc_out, LiquidCrystal lcd);
  
  void vaccineDisplayOut(int vacc_out, LiquidCrystal lcd);
  
  void completeDisplay(LiquidCrystal lcd, int total);

  //vaccine_out function that calcualtes and returns the closest number of vaccines withdrawable by the shipper (units of 11 doses) to the request of the user.
  int vaccine_out(int vacc_in, int total, int col);

  //rot function that calculates and returns the number of rotations required for the servos to complete the withdrawal.
  int rot(int vacc_out, int col);

  //layerEmpty function that checks if a layer is empty (no vaccines).
  bool layerEmpty(int col_count);

  //catchFull function that checks if the catch compartment is full (11 * 13 = 143 doses).
  bool catchFull(int rotation);

  //blueLED function that turns on or off the blue LED to represent the state of the retrieval compartment (on = retrieval compartment is opened, off = retreival compartment is closed).
  void blueLED(bool full, int pinNumber);
  
  //LogTime function that checks if the current time is 10min away from the last time the temperature was logged
  bool LogTime (long milli, long prevMilli, long period);
  
  // This function takes analogue input from the knob and returns the value.
  int measureVacc();

  // This function finds the number of vaccines required by the user (from knob input).
  int convertVacc(int vacc_analog, int pos_knob);
}

#endif /* FUNCTION_H_ */

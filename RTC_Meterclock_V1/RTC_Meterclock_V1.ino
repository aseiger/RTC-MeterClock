#include <Wire.h>

/* this program tests the Analog Panel Meter display




*/


#define hours_pin 11
#define minutes_pin 10
#define seconds_pin 9
#define increment_hours_pin 3
#define increment_minutes_pin 4
#define reset_seconds_pin 5
#define pps_pin 2
#define pps_out 13

#define sec_increment 4
#define min_increment 4
#define hour_increment 11

#define set_wait_time 2
#define set_exit_time 5

#define RTC_ADDR 0x68

#define DISPLAY_TIME 0
#define SET_HOURS 1
#define SET_MINUTES 2

unsigned volatile int hours = 23;
unsigned volatile int minutes = 59;
unsigned volatile int seconds = 59;

unsigned volatile int tic = 0;

unsigned int time_pressed = 0;
unsigned int time_released = 0;
unsigned int seconds_last = 0;

unsigned int wigglevalue = 60;

unsigned int state = DISPLAY_TIME;

void second_increment() {
  seconds++;
  if(seconds > 59){
    seconds = 0;
    minutes++;
    if(minutes > 59){
      minutes = 0;
      hours++;
      if(hours > 23){
        hours = 0;
      }
    }
  }
  //signal main program a tic has occured
  tic = 1;
}

void setup(){
  //write all outputs to max, for calibration
  analogWrite(hours_pin, hours_to_pwm(hours));
  analogWrite(minutes_pin, minutes_to_pwm(minutes));
  analogWrite(seconds_pin, minutes_to_pwm(seconds));
  //connect all inputs
  pinMode(increment_hours_pin, INPUT_PULLUP);
  pinMode(increment_minutes_pin, INPUT_PULLUP);
  pinMode(reset_seconds_pin, INPUT_PULLUP);
  pinMode(pps_out, OUTPUT);
  //PPS interrupt connect
  pinMode(pps_pin, INPUT_PULLUP);
  attachInterrupt(0, second_increment, RISING);
  delay(10000);
  Serial.begin(9600);
  hours = 0;
  minutes = 0;
  seconds = 0;
  Wire.begin();
  //get real time from RTC
  getTime();
  //enable PPS output
  clkInit();
}

void loop(){
  
    analogWrite(hours_pin, hours_to_pwm(hours));
    analogWrite(minutes_pin, minutes_to_pwm(minutes));
    analogWrite(seconds_pin, minutes_to_pwm(seconds));
    
    //increment hours if button pressed
    if(digitalRead(increment_hours_pin) == 0){
      delay(5);
      if(digitalRead(increment_hours_pin) == 0){
        while(digitalRead(increment_hours_pin) == 0);
        getTime();
        if(hours <= 22){
          hours++;
        } else {
          hours = 0;
        }
        setTime();
      }
    }
    
    //increment minutes if button pressed
    if(digitalRead(increment_minutes_pin) == 0){
      delay(5);
      if(digitalRead(increment_minutes_pin) == 0){
        while(digitalRead(increment_minutes_pin) == 0);
        getTime();
        if(minutes <= 58){
          minutes++;
        } else {
          minutes = 0;
        }
        setTime();
      }
    }
    
    //seconds if button pressed
    if(digitalRead(reset_seconds_pin) == 0){
      delay(5);
      if(digitalRead(reset_seconds_pin) == 0){
        while(digitalRead(reset_seconds_pin) == 0);
        getTime();
        seconds = 0;
        setTime();
      }
    }
    if(tic == 1){
      digitalWrite(pps_out,1);
      tic = 0;
    } else {
      digitalWrite(pps_out,0);
    }
    
    delay(10);
  
}

unsigned int hours_to_pwm(unsigned int hours) {
  return hours*hour_increment;
}

unsigned int minutes_to_pwm(unsigned int minutes){
  return minutes*min_increment;
}

unsigned int seconds_to_pwm(unsigned int seconds){
  return seconds*sec_increment;
}

byte bcdToDec(byte val)  {
// Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}

byte decToBcd(byte val){
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}

void getTime(){
  Wire.beginTransmission(RTC_ADDR);
  //reset register pointer to zero
  Wire.write(0x00);
  Wire.endTransmission();
  //read in the 7 bytes
  Wire.requestFrom(RTC_ADDR, 3);
  
  seconds = bcdToDec(Wire.read());
  minutes = bcdToDec(Wire.read());
  hours = bcdToDec(Wire.read() & 0b111111); //24 hour time
}

void setTime(){
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x00); //stop Oscillator

  Wire.write(decToBcd(seconds));
  Wire.write(decToBcd(minutes));
  Wire.write(decToBcd(hours));
  Wire.endTransmission();
}

void clkInit(){
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x07); //PPS register location
  
  Wire.write(0x10); //return to zero

  Wire.endTransmission();
}
  

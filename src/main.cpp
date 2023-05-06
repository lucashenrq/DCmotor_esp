#include <Arduino.h>
#include <math.h>
#include "RobojaxBTS7960.h"

#define Ts 44100
const int pulse_in_pin = 2;
int pulse_width = 1500; // In case there's no reading, the motor should stop
bool direction = 1;     // 1 represents forward
int intensity = 0;

const int forward_enable = 3;
const int reverse_enable = 4;
const int forward_pwm = 5;
const int reverse_pwm = 6;
const int r_current_sensor = A0;
const int L_current_sensor = A1;
#define DEBUG 0

unsigned long last_millis = 0; // just to time the serial prints
#define PRINT_INTERVAL 400

// Many of these pins don't really matter if we just want to control the motor
RobojaxBTS7960 motor(forward_enable, forward_pwm, r_current_sensor, reverse_enable, reverse_pwm, L_current_sensor, DEBUG);

void receiver_to_h_bridge(int pulse_width, bool &direction, int &intensity);

void setup()
{
  Serial.begin(9600);
  pinMode(pulse_in_pin, INPUT);
  motor.begin();
}

void loop()
{
  // Read the pulse_width, later use an actual interrupt
  pulse_width = pulseIn(pulse_in_pin, HIGH, Ts);

  // Convert
  receiver_to_h_bridge(pulse_width, direction, intensity);

  // Transmit command to motor
  motor.rotate(intensity, direction);

  // Print what's happening
  if (millis() >= last_millis + PRINT_INTERVAL)
  {
    last_millis = millis();
    Serial.print(pulse_width);
    Serial.print("\t");
    Serial.print(direction ? "FORWARD" : "REVERSE");
    Serial.print("\t");
    Serial.print(intensity);
    Serial.print("\n");
  }
}

void receiver_to_h_bridge(int pulse_width, bool &direction, int &intensity)
{
  // 1000 -> full reverse
  // 1500 -> stopped
  // 2000 -> full forward

  int tmp = 1500 - pulse_width;
  int abs_of_tmp = abs(tmp);
  int tmp_intensity;
  bool tmp_direction;
  // check if the readout makes sense, if it doesn't, make the motor stop
  if (abs_of_tmp > 500)
  {
    tmp_intensity = 0;
    tmp_direction = 0;
  }
  else
  {
    tmp_intensity = map(abs_of_tmp, 0, 500, 0, 100);
    tmp_intensity = constrain(intensity, 0, 255);
    tmp_direction = tmp > 0;
  }
  // Attribute the values, since those are references, the variables will be
  // modified without a return value for the function
  intensity = tmp_intensity;
  direction = tmp_direction;
}
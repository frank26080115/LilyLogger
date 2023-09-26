#include "S3Servo.h"

/**
 * @brief   attaches the given pin, channel, freq, resolution
 * @param   @pin : servo pin
 *          @channel : channel of pwm
 *          @freq : frequency of pwm
 *          @resolution : range is 1-14 bits (1-20 bits for ESP32)
 * @retval  None
**/

void S3Servo::attach(int pin, int channel, int freq, int resolution) {
  _channel = channel;
  if(channel > 15) channel = 15;
  ledcSetup(_channel, freq, resolution);
  ledcAttachPin(pin, channel);
  ledcWrite(_channel, FIRSTDUTY);
}

/**
 * @brief   writes servo value 0-180 as degree
 * @param   @value: servo value 0-180 as degree
 * @retval  None
**/

void S3Servo::write(int value) {
  if(value < 0) value = 0;
  if(value > 180) value = 180;
  int servoValue = (value - SERVOMIN) * (DUTYCYLEMAX - DUTYCYLEMIN) / (SERVOMAX - SERVOMIN) + DUTYCYLEMIN;
  ledcWrite(_channel, servoValue);
}

void S3Servo::writeMicroseconds(int value) {
  if (value < 0) value = 0;
  if (value > 3000) value = 3000;
  value = map(value, 0, 3000, 0, 180);
  this->write(value);
}

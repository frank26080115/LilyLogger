#ifndef _S3SERVO_H
#define _S3SERVO_H

#include <Arduino.h>

#define DELAYMS 2000

#define PWMFREQ 50
#define PWMCHANNEL 0
#define PWMRESOLUTION 12
#define FIRSTDUTY 0

#define SERVOMIN 0
#define SERVOMAX 180
#define DUTYCYLEMIN 100
#define DUTYCYLEMAX 500

class S3Servo {
public:
  void attach(int pin, int channel = PWMCHANNEL, int freq = PWMFREQ, int resolution = PWMRESOLUTION);
  void write(int value);
  void writeMicroseconds(int value);
private:
  int _channel;
};

#endif

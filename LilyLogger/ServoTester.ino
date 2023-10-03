#include "LilyLogger.h"
#include "pin_config.h"
#include <fadc.h>
#include <S3Servo.h>

extern int8_t digitalPinToAnalogChannel(uint8_t pin);

S3Servo servo;
uint32_t servo_us;
int8_t adc_chan;

void servotester_init()
{
    servo.attach(PIN_SERVO_PWM);
    servo.writeMicroseconds(servo_us = 1000);
    adc_chan = digitalPinToAnalogChannel(PIN_SERVO_POT);
    fadcStart(adc_chan);
}

void servotester_task()
{
    if (fadcBusy() == false) {
        uint16_t x = fadcApply(fadcResult() << FADC_SHIFT);
        int deadzone = 32;
        servo_us = map_rounded(x, deadzone, 4095 - deadzone, 1000, 2000, true);
        servo.writeMicroseconds(servo_us);
        fadcStart(adc_chan); // do the next read
    }
}

uint32_t servotester_get()
{
    return servo_us;
}

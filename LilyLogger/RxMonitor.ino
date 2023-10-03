#include "LilyLogger.h"
#include "pin_config.h"

volatile uint32_t rxmon_becfault = 0;      // zero is no-fault, otherwise, millisecond timestamp of last event
bool rxmon_becready = 0;                   // disable feature until BEC is actually detected
volatile uint32_t rxmon_pulseTime = 0;     // ms timestamp of last falling edge
volatile uint32_t rxmon_pulseTimeUs = 0;   // us timestamp of last rising edge
volatile uint32_t rxmon_pulseWidth = 0;    // pulse width in us, calculated upon falling edge
volatile uint32_t rxmon_pulsePeriod = 0;   // pulse period between consecutive falling edge
volatile bool rxmon_pulseActive = false;   // valid pulse has been detected, feature is enabled

void IRAM_ATTR rxmon_intr_bec();
void IRAM_ATTR rxmon_intr_pulse();

void rxmon_init()
{
    pinMode(PIN_RXMON_BEC, INPUT_PULLDOWN);
    pinMode(PIN_RXMON_PULSE, INPUT_PULLDOWN);
    attachInterrupt(PIN_RXMON_BEC, rxmon_intr_bec, FALLING);
    attachInterrupt(PIN_RXMON_PULSE, rxmon_intr_pulse, CHANGE);
}

uint32_t rxmon_hasBecFault()
{
    if (rxmon_becready == false) {
        // feature disabled, do not report fault
        return 0;
    }
    return rxmon_becfault;
}

void rxmon_clrBecFault()
{
    rxmon_becfault = 0;
}

void rxmon_task()
{
    if (digitalRead(PIN_RXMON_BEC) != LOW) {
        // BEC detected, enable this feature
        rxmon_becready = true;
    }
}

void rxmon_pulseGet(uint32_t* period, uint32_t* width)
{
    if (rxmon_pulseActive == false) {
        // feature not active
        if (period) {
            *period = 0;
        }
        if (width) {
            *width = 0;
        }
        return;
    }
    bool timeout = false;
    uint32_t now = millis();
    if ((now - rxmon_pulseTime) >= 500) {
        timeout = true;
    }
    if (period)
    {
        if (timeout) {
            *period = 9999;
        }
        else {
            *period = rxmon_pulsePeriod;
        }
    }
    if (width)
    {
        *width = rxmon_pulseWidth;
    }
}

void IRAM_ATTR rxmon_intr_bec()
{
    rxmon_becfault = millis();
}

void IRAM_ATTR rxmon_intr_pulse()
{
    if (digitalRead(PIN_RXMON_PULSE) == LOW)
    {
        uint32_t ms = millis();
        rxmon_pulsePeriod = ms - rxmon_pulseTime;
        rxmon_pulseTime = ms;
        uint32_t us = micros();
        if (us > rxmon_pulseTimeUs) {
            rxmon_pulseWidth = us - rxmon_pulseTimeUs;
        }
        if (rxmon_pulsePeriod >= 5 && rxmon_pulsePeriod <= 50) {
            rxmon_pulseActive = true;
        }
    }
    else
    {
        rxmon_pulseTimeUs = micros();
    }
}

#include "LilyLogger.h"
#include <Ina219.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;
extern CurrentSensorIna219 ina;


uint16_t current_to_pixels(int16_t x)
{
    volatile double xd = x + 1;
    if (xd < 1) {
        return 0; // do not show reverse current
    }
    volatile double h = tft.height() - 1;
    xd = h * log10(xd);
    uint16_t y = lround(xd / log10(0x1000));
    return tft.height() - y;
}

uint32_t current_to_mA(int16_t x)
{
    /*
    Shunt being used is 50A at 75mV
    The INA219 is configured for 80mV
    4095 * (75/80) = 3839
    */
    return map_rounded(x, 0, 3839, 0, 50000, false);
}

uint16_t voltage_to_pixels(int16_t x)
{
    return tft.height() - map_rounded(x, 0, 4095, 0, tft.height() - 1, true);
}

uint32_t voltage_to_mV(int16_t x)
{
    return map_rounded(x, 0, 4095, 0, 16000, false);
}

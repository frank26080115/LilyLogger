#include "LilyLogger.h"
#include <Ina219.h>

extern CurrentSensorIna219* ina;
uint16_t ina_cfg = INA219_DEFAULT_CFG;

uint16_t current_to_pixels(int16_t x)
{
    volatile double xd = x + 1;
    if (xd < 1) {
        return 0; // do not show reverse current
    }
    volatile double h = gui_screenHeight - 1;
    xd = h * log10(xd);
    uint16_t y = lround(xd / log10(nvmsettings.current_pix_scale));
    return gui_screenHeight - y;
}

uint32_t current_to_mA(int16_t x)
{
    return map_rounded(x, 0, nvmsettings.current_div, 0, nvmsettings.current_multi, false);
}

uint16_t voltage_to_pixels(int16_t x)
{
    return gui_screenHeight - map_rounded(x, 0, nvmsettings.voltage_pix_scale, 0, gui_screenHeight - 1, true);
}

uint32_t voltage_to_mV(int16_t x)
{
    return map_rounded(x, 0, 0x1000 - 1, 0, nvmsettings.voltage_mv_scale, false);
}

#include "LilyLogger.h"
#include "pin_config.h"

volatile SemaphoreHandle_t tacho_mutex;
void IRAM_ATTR tacho_intr();

void tacho_init()
{
    tacho_mutex = xSemaphoreCreateMutex();
    pinMode(PIN_TACHO, (nvmsettings.tacho_edge == FALLING) ? INPUT_PULLUP : INPUT_PULLDOWN);
    attachInterrupt(PIN_TACHO, tacho_intr, nvmsettings.tacho_edge);
}

volatile uint32_t tacho_last_time = 0;
volatile uint32_t tacho_last_time_ms = 0;
volatile uint32_t tacho_cnt = 0;
volatile uint32_t tacho_last_span = 0;
volatile uint32_t tacho_span_min = 0;
volatile uint32_t tacho_span_max = 0;
volatile uint32_t tacho_span_sum = 0;

void IRAM_ATTR tacho_intr()
{
    uint32_t t = (uint32_t)micros();
    tacho_cnt += 1;

    if (t < tacho_last_time) {
        // do nothing if overflow detected
        tacho_last_time = t;
        return;
    }
    if (tacho_last_time == 0) {
        // do nothing on first tick
        tacho_last_time = t;
        return;
    }
    uint32_t span = t - tacho_last_time;
    tacho_last_time = t;
    if (span < 500) {
        // reject impossibly short pulse
        return;
    }
    tacho_last_time_ms = millis();
    tacho_last_span = span;
    tacho_span_sum += span;
    tacho_span_min = tacho_last_span < tacho_span_min ? tacho_last_span : tacho_span_min;
    tacho_span_max = tacho_last_span > tacho_span_max ? tacho_last_span : tacho_span_max;
}

uint32_t tacho_calcRpm(uint32_t span)
{
    double spand = span;
    double rps = 1000000;
    rps /= spand;
    double rpm = rps * 60;
    return (uint32_t)lround(rpm);
}

tacho_data_t tacho_100msTask()
{
    static tacho_data_t r;

    portDISABLE_INTERRUPTS();
    xSemaphoreTake(tacho_mutex, portMAX_DELAY);

    r.rpm_max = tacho_calcRpm(tacho_span_min);
    r.rpm_min = tacho_calcRpm(tacho_span_max);
    tacho_span_min = 0xFFFFFFFF;
    tacho_span_max = 0;

    uint32_t now = millis();
    if ((now - tacho_last_time_ms) > 1000) {
        // timeout waiting for pulse
        r.rpm_avg = 0;
    }
    else {
        if (tacho_cnt >= 10) {
            uint32_t span_avg = (tacho_span_sum + (tacho_cnt / 2)) / tacho_cnt;
            tacho_cnt = 0;
            tacho_span_sum = 0;
            r.rpm_avg = tacho_calcRpm(span_avg);
        }
    }

    xSemaphoreGive(tacho_mutex);
    portENABLE_INTERRUPTS();

    return r;
}
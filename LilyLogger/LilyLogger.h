#ifndef _LILYLOGGER_H_
#define _LILYLOGGER_H_

#include <Arduino.h>

typedef struct
{
    uint32_t rpm_avg;
    uint32_t rpm_max;
    uint32_t rpm_min;
}
tacho_data_t;

typedef struct
{
    // this structure only cares about screen coordinates
    // calculations are only done once, for performance reasons
    uint16_t current_max;
    uint16_t current_avg;
    uint16_t current_min;
    uint16_t voltage_max;
    uint16_t voltage_avg;
    uint16_t voltage_min;
    #ifdef ENABLE_PLOT_RPM
    uint16_t speed_max;
    uint16_t speed_avg;
    uint16_t speed_min;
    #endif
    bool bec_fault;
}
plot_data_t;

typedef struct 
{
    uint32_t timestamp;
    uint32_t current_avg;
    uint32_t current_max;
    uint32_t current_min;
    uint32_t voltage_avg;
    uint32_t voltage_max;
    uint32_t voltage_min;
    uint32_t rpm_avg;
    uint32_t rpm_max;
    uint32_t rpm_min;
    uint32_t servo_pwm;
    uint32_t bec_fault;
}
log_data_t;

typedef struct
{
    int32_t ina_cfg;
    int32_t current_multi;
    int32_t current_div;
    int32_t current_pix_scale;
    int32_t voltage_pix_scale;
    int32_t voltage_mv_scale;
    int32_t tacho_edge;
}
nvmsettings_t;

extern nvmsettings_t nvmsettings;
extern uint16_t gui_screenWidth, gui_screenHeight;

extern void usbmsc_init();
extern void nvmsettings_init();
extern void gui_init();
extern void gui_bootmsg(const char*);
extern void tacho_init();
extern tacho_data_t tacho_100msTask();
extern void plot_push(plot_data_t* x);
extern void plot_draw();
extern bool datalog_openNextFile();
extern void datalog_stop();
extern char* datalog_fname();
extern bool datalog_isOpen();
extern void datalog_write(log_data_t* data, bool flush);
extern void servotester_init();
extern void servotester_task();
extern uint32_t servotester_get();
extern void rxmon_init();
extern void rxmon_task();
extern void rxmon_pulseGet(uint32_t* period, uint32_t* width);
extern uint32_t rxmon_hasBecFault();
extern void rxmon_clrBecFault();

extern int32_t map_rounded(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max, bool limit);
extern int32_t div_rounded(const int n, const int d);

extern uint16_t current_to_pixels(int16_t x);
extern uint32_t current_to_mA(int16_t x);
extern uint16_t voltage_to_pixels(int16_t x);
extern uint32_t voltage_to_mV(int16_t x);

#endif // _LILYLOGGER_H_

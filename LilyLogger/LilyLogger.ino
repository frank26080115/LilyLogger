#include "LilyLogger.h"
#include "pin_config.h"
#include "FS.h"
#include "FFat.h"
#include <Ina219.h>
#include <TFT_eSPI.h>
#include <fadc.h>
#include <S3Servo.h>

TaskHandle_t Task1;
void Task1code(void* pvParameters);

extern TFT_eSPI tft;
extern TFT_eSprite sprite;

CurrentSensorIna219 ina = CurrentSensorIna219((gpio_num_t)PIN_IIC_SDA, (gpio_num_t)PIN_IIC_SCL);

extern S3Servo servo;

void setup()
{
    pinMode(PIN_BUTTON_1, INPUT_PULLUP);
    pinMode(PIN_BUTTON_2, INPUT_PULLUP);
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    gui_init(); // initialize LCD first in order to display error messages

    if (!FFat.begin())
    {
        // must fix this error manually 
        gui_bootmsg("FFat Error");

        // do a forever loop, but allow the user to reformat the drive by holding down both buttons for a few seconds
        uint32_t t = 0;
        while (true)
        {
            yield();
            uint32_t now = millis();
            if (digitalRead(PIN_BUTTON_1) == LOW && digitalRead(PIN_BUTTON_2) == LOW && t == 0) {
                // timer not started, start the timer to check button hold
                t = now;
            }
            else if (digitalRead(PIN_BUTTON_1) == LOW && digitalRead(PIN_BUTTON_2) == LOW && t != 0) {
                // timer already started, check if button is being held
                if ((now - t) >= 5000) {
                    FFat.format();
                    gui_bootmsg("FFat Reformatted");
                    while (true) {
                        yield();
                    }
                }
            }
            else if (digitalRead(PIN_BUTTON_1) != LOW || digitalRead(PIN_BUTTON_2) != LOW) {
                t = 0; // button released, stop the timer
            }
        }
    }

    // press this button during boot will lock the device into USB flash drive mode
    if (digitalRead(PIN_BUTTON_2) == LOW)
    {
        tft.drawString("USB Flash Drive Mode", 5, 5, 4);
        usbmsc_init();
        while (true) {
            yield();
        }
    }

    Serial.begin(115200); // this should be using USB CDC

    gui_bootmsg("Hello!");

    // the I2C thread is on the other core
    // Wi-Fi is not used so the core should be pretty free
    // the I2C calls are blocking, so using another core is how we continue to sample while GUI updates and flash is written
    xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    2048,        /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    ((xPortGetCoreID() == 0) ? 1 : 0) /* pin task to core */ 
                    );
}

uint32_t last_tick_time = 0; // keep frame rate
bool btnlatch_1 = false;     // detect button release
bool btnlatch_2 = false;     // detect button release
uint8_t disp_font = 0;       // user selected display font

void loop()
{
    // the screen refresh rate is only 10 FPS
    bool should_run = false;
    uint32_t now = millis();
    if ((now - last_tick_time) >= 100) {
        if ((now - last_tick_time) >= 200) {
            last_tick_time = now; // too far ahead, just reset the timer and miss a frame
        }
        else {
            last_tick_time += 100; // catch up
        }
        should_run = true;
    }
    if (should_run == false) {
        // do nothing, let other threads run
        vTaskDelay(0);
        return;
    }

    // gather data
    current_sensor_results_t sensor_results = ina.get(true);
    tacho_data_t tacho_data = tacho_100msTask();

    plot_data_t pltdata;
    log_data_t logdata;

    // convert data to pixel coordinates for plot
    pltdata.current_avg = current_to_pixels(sensor_results.current_avg);
    pltdata.current_min = current_to_pixels(sensor_results.current_min);
    pltdata.current_max = current_to_pixels(sensor_results.current_max);
    pltdata.voltage_avg = voltage_to_pixels(sensor_results.voltage_avg);
    pltdata.voltage_max = voltage_to_pixels(sensor_results.voltage_max);
    pltdata.voltage_min = voltage_to_pixels(sensor_results.voltage_min);
    #ifdef ENABLE_PLOT_RPM
    pltdata.speed_avg = tacho_data.rpm_avg;
    pltdata.speed_max = tacho_data.rpm_max;
    pltdata.speed_min = tacho_data.rpm_min;
    #endif
    plot_push(&pltdata);

    // convert to human units for data logging and display
    logdata.timestamp   = millis();
    logdata.current_avg = current_to_mA(sensor_results.current_avg);
    logdata.current_min = current_to_mA(sensor_results.current_min);
    logdata.current_max = current_to_mA(sensor_results.current_max);
    logdata.voltage_avg = voltage_to_mV(sensor_results.voltage_avg);
    logdata.voltage_max = voltage_to_mV(sensor_results.voltage_max);
    logdata.voltage_min = voltage_to_mV(sensor_results.voltage_min);
    logdata.rpm_avg     = tacho_data.rpm_avg;
    logdata.rpm_max     = tacho_data.rpm_max;
    logdata.rpm_min     = tacho_data.rpm_min;
    logdata.servo_pwm   = servotester_get();

    sprite.fillSprite(TFT_BLACK);
    plot_draw();
    sprite.setTextFont(0);
    sprite.setTextColor(TFT_RED, TFT_BLACK, true);
    int16_t y = 3;
    int16_t x_margin = 3;
    int16_t line_spacing = 1;
    int16_t font_height;
    char strbuf[256];
    if (datalog_isOpen()) {
        sprintf(strbuf, "REC FILE: %s", datalog_fname());
        datalog_write(&logdata, false);
    }
    else {
        sprintf(strbuf, "no file logging");
    }

    // write out file status on screen
    sprite.drawString(strbuf, x_margin, y);
    // write out data on the screen
    y += sprite.fontHeight() + line_spacing;
    sprite.setTextFont(disp_font);
    font_height = sprite.fontHeight();
    sprite.setTextColor(TFT_RED, TFT_BLACK, true);
    sprintf(strbuf, "I: %0.1f A", ((float)logdata.current_avg) / (float)1000.0);
    sprite.drawString(strbuf, x_margin, y);
    y += font_height + line_spacing;
    sprite.setTextColor(TFT_GREEN, TFT_BLACK, true);
    sprintf(strbuf, "V: %0.1f V", ((float)logdata.voltage_avg) / (float)1000.0);
    sprite.drawString(strbuf, x_margin, y);
    y += font_height + line_spacing;
    sprite.setTextColor(TFT_YELLOW, TFT_BLACK, true);
    sprintf(strbuf, "Tacho: %u RPM", logdata.rpm_avg);
    sprite.drawString(strbuf, x_margin, y);
    y += font_height + line_spacing;
    sprite.setTextColor(logdata.servo_pwm <= 1050 ? TFT_WHITE : (logdata.servo_pwm <= 1500 ? TFT_YELLOW : TFT_ORANGE), TFT_BLACK, true);
    sprintf(strbuf, "Servo: %u us", logdata.servo_pwm);
    sprite.drawString(strbuf, x_margin, y);

    // show to serial port if available
    Serial.printf("%u, ", logdata.timestamp);
    Serial.printf("%u, %u, %u "
                  "%u, %u, %u "
                  "%u, %u, %u "
                  "%u"
        , logdata.current_avg
        , logdata.current_min
        , logdata.current_max
        , logdata.voltage_avg
        , logdata.voltage_min
        , logdata.voltage_max
        , logdata.rpm_avg
        , logdata.rpm_min
        , logdata.rpm_max
        , logdata.servo_pwm
        );
    Serial.println();

    sprite.pushSprite(0, 0); // actually draw to LCD

    // button 2 press changes the font
    if (digitalRead(PIN_BUTTON_2) == LOW && btnlatch_2 == false) {
        btnlatch_2 = true;
        disp_font = (disp_font + 1) % 5;
    }
    else if (digitalRead(PIN_BUTTON_2) != LOW) {
        btnlatch_2 = false;
    }

    // button 1 press starts or stops data logging
    if (digitalRead(PIN_BUTTON_1) == LOW && btnlatch_1 == false) {
        btnlatch_1 = true;
        if (datalog_isOpen()) {
            datalog_stop();
        }
        else {
            datalog_openNextFile();
        }
    }
    else if (digitalRead(PIN_BUTTON_1) != LOW) {
        btnlatch_1 = false;
    }
}

void Task1code(void* pvParameters)
{
    ina.begin();
    servotester_init();
    while (true)
    {
        ina.task();         // this is blocking
        servotester_task(); // this is none blocking, very quick and low priority
        vTaskDelay(0);
    }
}

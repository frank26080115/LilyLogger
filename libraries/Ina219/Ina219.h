#ifndef _INA219_H_
#define _INA219_H_

#include <Arduino.h>
#include "driver/i2c.h"
#include "ina219_consts.h"

enum
{
    CSINA219_STATEMACH_INIT,
    CSINA219_STATEMACH_READ,
};

typedef struct
{
    int16_t current_avg;
    int16_t current_max;
    int16_t current_min;
    int16_t voltage_avg;
    int16_t voltage_max;
    int16_t voltage_min;
}
current_sensor_results_t;

class CurrentSensorIna219
{
    public:
        CurrentSensorIna219(gpio_num_t _pin_sda, gpio_num_t _pin_scl, uint16_t _cfg_reg
            = 0
            | INA219_CONFIG_BVOLTAGERANGE_16V
            | INA219_CONFIG_GAIN_2_80MV
            | INA219_CONFIG_BADCRES_12BIT
            | INA219_CONFIG_SADCRES_12BIT_1S_532US
            | INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS
            , uint8_t _i2c_num = 0
        ) {
            pin_sda = _pin_sda;
            pin_scl = _pin_scl;
            cfg_reg = _cfg_reg;
            i2c_num = _i2c_num;
        };

        void begin(bool hw = true) {
            if (hw) {
                i2c_config_t conf = {
                    .mode = I2C_MODE_MASTER,
                    .sda_io_num = pin_sda,
                    .scl_io_num = pin_scl,
                    .sda_pullup_en = true,
                    .scl_pullup_en = true,
                };
                conf.master.clk_speed = 400000;

                i2c_param_config(i2c_num, &conf);
                i2c_driver_install(i2c_num, conf.mode, 0, 0, 0);
            }
            statemachine = CSINA219_STATEMACH_INIT;
            mutex = xSemaphoreCreateMutex();
        };

        void task(void) {
            switch (statemachine)
            {
                case CSINA219_STATEMACH_INIT:
                    {
                        xSemaphoreTake(mutex, portMAX_DELAY);
                        tx_buff[0] = INA219_REG_CONFIG;
                        tx_buff[1] = (uint8_t)( cfg_reg       & 0xFF);
                        tx_buff[2] = (uint8_t)((cfg_reg >> 8) & 0xFF);
                        esp_err_t err = i2c_master_write_to_device(i2c_num, INA219_ADDRESS, tx_buff, 3, 1000 / portTICK_PERIOD_MS);
                        xSemaphoreGive(mutex);
                        if (err == ESP_OK)
                        {
                            get(true); // resets all values
                            statemachine = CSINA219_STATEMACH_READ;
                        }
                    }
                    break;
                case CSINA219_STATEMACH_READ:
                    {
                        xSemaphoreTake(mutex, portMAX_DELAY);
                        tx_buff[0] = INA219_REG_SHUNTVOLTAGE;
                        esp_err_t err = i2c_master_write_read_device(i2c_num, INA219_ADDRESS, tx_buff, 1, rx_buff, 4, 1000 / portTICK_PERIOD_MS);
                        if (err == ESP_OK)
                        {
                            procResult();
                        }
                        else
                        {
                            statemachine = CSINA219_STATEMACH_INIT;
                        }
                        xSemaphoreGive(mutex);
                    }
                    break;
                default:
                    statemachine = CSINA219_STATEMACH_INIT;
                    break;
            }
        };

        current_sensor_results_t get(bool clr)
        {
            xSemaphoreTake(mutex, portMAX_DELAY);
            current_sensor_results_t r;
            r.current_avg = (current_sum + (current_cnt / 2)) / current_cnt;
            r.voltage_avg = (voltage_sum + (voltage_cnt / 2)) / voltage_cnt;
            r.current_max = current_max;
            r.current_min = current_min;
            r.voltage_max = voltage_max;
            r.voltage_min = voltage_min;
            if (clr)
            {
                current_min = current_max;
                voltage_min = voltage_max;
                current_max = 0;
                voltage_max = 0;
                current_sum = 0;
                voltage_sum = 0;
                current_cnt = 0;
                voltage_cnt = 0;
            }
            xSemaphoreGive(mutex);
            return r;
        };

    private:
        uint8_t statemachine;

        gpio_num_t pin_sda, pin_scl;
        uint16_t cfg_reg;
        uint8_t i2c_num;

        uint32_t c_ma_max, v_mv_max;

        SemaphoreHandle_t mutex;

        uint8_t tx_buff[8];
        uint8_t rx_buff[16];

        uint32_t current_sum, current_cnt;
        uint32_t voltage_sum, voltage_cnt;
        uint32_t current_max, current_min;
        uint32_t voltage_max, voltage_min;

        void procResult()
        {
            uint16_t* ptr16 = (uint16_t*)rx_buff;
            uint16_t current = ptr16[0];
            uint16_t voltage = ptr16[1];
            current_sum += current;
            current_cnt += 1;
            voltage_sum += voltage;
            voltage_cnt += 1;
            current_max = current > current_max ? current : current_max;
            voltage_max = voltage > voltage_max ? voltage : voltage_max;
            current_min = current < current_min ? current : current_min;
            voltage_min = voltage < voltage_min ? voltage : voltage_min;
        };
};

#endif // _INA219_H_
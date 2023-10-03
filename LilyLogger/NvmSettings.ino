#include "LilyLogger.h"
#include <Ina219.h>
#include "FS.h"
#include "FFat.h"
#include <RoachCmdLine.h>

#define CFG_FILE_NAME    "/config.txt"

extern uint16_t ina_cfg;
nvmsettings_t nvmsettings;
extern RoachCmdLine cmdline;

void nvmsettings_init()
{
    nvmsettings_default();
    nvmsettings_load();
}

void nvmsettings_load()
{
    File cfg_file = FFat.open(CFG_FILE_NAME);
    if (!cfg_file) {
        Serial.println("ERR: failed to open config.txt for loading");
        return;
    }

    bool is_ending = false;
    char str[128];
    while (true) {
        int rlen = cfg_file.readBytesUntil('\n', (uint8_t*)str, 128);
        if (rlen <= 0) {
            break;
        }
        else if (rlen <= 2 && is_ending == false) {
            is_ending = true;
        }
        else if (rlen <= 2 && is_ending) {
            break;
        }
        else if (rlen > 2) {
            is_ending = false;
        }
        str[rlen] = 0;
        cmdline.sideinput_writes((const char*)str);
    }

    cfg_file.close();
}

void nvmsettings_default()
{
    nvmsettings.ina_cfg           = INA219_DEFAULT_CFG;
    nvmsettings.current_multi     = 50000;  // Shunt reference current in mA
    nvmsettings.current_div       = 3839;   // Shunt being used is 50A at 75mV // The INA219 is configured for 80mV // so 4095 * (75/80) = 3839
    nvmsettings.current_pix_scale = 0x1000;
    nvmsettings.voltage_pix_scale = 0x1000 - 1;
    nvmsettings.voltage_mv_scale  = 16000;
    nvmsettings.tacho_edge        = RISING;
}

void nvmsettings_save()
{
    if (FFat.remove(CFG_FILE_NAME)) {
        Serial.println("config file delete successful");
    }
    else {
        Serial.println("ERR: config file delete failed");
    }

    File cfg_file = FFat.open(CFG_FILE_NAME, FILE_WRITE);
    if (!cfg_file) {
        Serial.println("ERR: failed to open config.txt for saving");
        return;
    }
    cfg_file.printf("setcfg ina_cfg 0x%04X\r\n", nvmsettings.ina_cfg);
    cfg_file.printf("setcfg current_multi %u\r\n", nvmsettings.current_multi);
    cfg_file.printf("setcfg current_div %u\r\n", nvmsettings.current_div);
    cfg_file.printf("setcfg current_pix_scale %u\r\n", nvmsettings.current_pix_scale);
    cfg_file.printf("setcfg voltage_pix_scale %u\r\n", nvmsettings.voltage_pix_scale);
    cfg_file.printf("setcfg voltage_mv_scale %u\r\n", nvmsettings.voltage_mv_scale);
    cfg_file.printf("setcfg tacho_edge %u\r\n", nvmsettings.tacho_edge);
    cfg_file.printf("\r\n\r\n\r\n");
    cfg_file.close();
}

#define CHAR_IS_WS(x) ((x) == ' ' || (x) == '\r' || (x) == '\n' || (x) == '\t')

void nvmsettings_parse(char* txt)
{
    int slen = strlen(txt);
    int i;
    int start_i, start_j;
    // trim start
    for (i = 0; i < slen; i++) {
        if (CHAR_IS_WS(txt[i]) == false) {
            start_i = i;
            break;
        }
    }
    // find end of first word
    for (; i < slen; i++) {
        if (CHAR_IS_WS(txt[i])) {
            txt[i] = 0;
            break;
        }
    }
    // find start of second word
    for (; i < slen; i++) {
        if (CHAR_IS_WS(txt[i]) == false) {
            start_j = i;
            break;
        }
    }
    // find end of second word
    for (; i < slen; i++) {
        if (CHAR_IS_WS(txt[i])) {
            txt[i] = 0;
            break;
        }
    }
    char* word1 = (char*)&(txt[start_i]);
    char* word2 = (char*)&(txt[start_j]);

    int32_t x;
    if (word2[0] == '0' && (word2[1] == 'x' || word2[1] == 'X')) {
        x = strtol(&(word2[2]), NULL, 16);
    }
    else {
        x = strtol(&(word2[0]), NULL, 10);
    }

    if (strcmp("current_multi", word1) == 0) {
        nvmsettings.current_multi = x;
    }
    else if (strcmp("current_div", word1) == 0) {
        nvmsettings.current_div = x;
    }
    else if (strcmp("ina_cfg", word1) == 0) {
        nvmsettings.ina_cfg = x;
    }
    else if (strcmp("current_pix_scale", word1) == 0) {
        nvmsettings.current_pix_scale = x;
    }
    else if (strcmp("voltage_pix_scale", word1) == 0) {
        nvmsettings.voltage_pix_scale = x;
    }
    else if (strcmp("voltage_mv_scale", word1) == 0) {
        nvmsettings.voltage_mv_scale = x;
    }
    else if (strcmp("tacho_edge", word1) == 0) {
        nvmsettings.tacho_edge = x;
    }
    else {
        Serial.printf("ERR: unknown setting name \"%s\" -> \"%s\" = %d\r\n", word1, word2, x);
    }
}

void nvmdebug_func(void* cmd, char* argstr, Stream* stream)
{
    File cfg_file = FFat.open(CFG_FILE_NAME);
    if (!cfg_file) {
        Serial.println("ERR: failed to open config.txt for debugging");
        return;
    }

    char str[128];
    while (true) {
        int rlen = cfg_file.readBytesUntil('\n', (uint8_t*)str, 128);
        if (rlen <= 0) {
            break;
        }
        str[rlen] = 0;
        Serial.println(str);
    }

    cfg_file.close();
}

void factory_reset_func(void* cmd, char* argstr, Stream* stream)
{
    if (FFat.remove(CFG_FILE_NAME)) {
        Serial.println("factory reset delete successful");
    }
    else {
        Serial.println("ERR: factory reset delete failed");
    }
}

void nvmload_func(void* cmd, char* argstr, Stream* stream)
{
    nvmsettings_init();
}

void nvmsave_func(void* cmd, char* argstr, Stream* stream)
{
    nvmsettings_save();
}

void setcfg_func(void* cmd, char* argstr, Stream* stream)
{
    nvmsettings_parse(argstr);
}

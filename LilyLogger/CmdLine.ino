#include "LilyLogger.h"
#include <RoachCmdLine.h>

const cmd_def_t cmds[] = {
    { "factoryreset", factory_reset_func},
    { "nvmdebug"    , nvmdebug_func},
    { "nvmload"     , nvmload_func},
    { "nvmsave"     , nvmsave_func},
    { "setcfg"      , setcfg_func },
    { "echo"        , echo_func },
    { "reboot"      , reboot_func },
    { "usbmsd"      , usbmsd_func },
    { "usbmsc"      , usbmsd_func },
    { "", NULL }, // end of table
};

RoachCmdLine cmdline(&Serial, (cmd_def_t*)cmds, false, (char*)">>>", (char*)"???", true, 512);

void echo_func(void* cmd, char* argstr, Stream* stream)
{
    stream->println(argstr);
}

void reboot_func(void* cmd, char* argstr, Stream* stream)
{
    stream->println("rebooting...\r\n\r\n");
    delay(100);
    ESP.restart();
}

void usbmsd_func(void* cmd, char* argstr, Stream* stream)
{
    stream->printf("presenting USB MSD\r\n");
    delay(100);
    usbmsc_init();
    while (true) {
        vTaskDelay(0);
    }
}

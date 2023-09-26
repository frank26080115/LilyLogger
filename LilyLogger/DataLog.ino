#include "LilyLogger.h"
#include "FS.h"
#include "FFat.h"

int log_filename_i = 0;
File log_file;
bool log_file_open = false;
char log_file_name[32];

bool datalog_openNextFile()
{
    if (log_file_open) {
        log_file_open = false;
        log_file.close();
        log_file_name[0] = 0;
    }

    char fname[32];
    log_filename_i++;
    for (; ; log_filename_i++) {
        sprintf(fname, "/log-%05u.bin", log_filename_i);
        File check_fexists = FFat.open(fname);
        if (!check_fexists) {
            check_fexists.close();
            log_file = FFat.open(fname, FILE_WRITE);
            if (!log_file) {
                Serial.printf("ERR: failed to open %s for writing\r\n", fname);
            }
            else {
                Serial.printf("Opened %s for writing\r\n", fname);
                log_file_open = true;
                strcpy(log_file_name, fname);
                return true;
            }
        }
        else {
            check_fexists.close();
        }
    }
    Serial.printf("ERR: failed to open any file for writing\r\n");
    log_file_open = false;
}

void datalog_stop()
{
    if (log_file_open) {
        log_file_open = false;
        log_file.close();
        log_file_name[0] = 0;
    }
}

char* datalog_fname()
{
    return log_file_name;
}

bool datalog_isOpen()
{
    return log_file_open;
}

void datalog_write(log_data_t* data, bool flush)
{
    log_file.write((const uint8_t*)data, sizeof(log_data_t));
    if (flush) {
        log_file.flush();
    }
}

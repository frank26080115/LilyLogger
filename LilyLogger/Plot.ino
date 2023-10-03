#include "LilyLogger.h"
#include <TFT_eSPI.h>

extern TFT_eSPI tft;
extern TFT_eSprite sprite;

#define PLOT_LENGTH 320

plot_data_t history[PLOT_LENGTH];
uint32_t history_idx = 0;

#ifdef ENABLE_PLOT_RPM
// used to auto-scale RPM plot
uint32_t plot_speed_max = 0;
uint32_t plot_speed_max_1000 = 0;
#endif

void plot_push(plot_data_t* x)
{
    plot_data_t* ptr = &(history[history_idx]);
    memcpy(ptr, x, sizeof(plot_data_t));
    history_idx = (history_idx + 1) % PLOT_LENGTH;
    #ifdef ENABLE_PLOT_RPM
    if (x->speed_max > plot_speed_max) {
        // set new max for auto-scaling
        plot_speed_max = x->speed_max;
        plot_speed_max_1000 = (plot_speed_max + 1) * 1000;
    }
    else {
        // decay the auto-scaling, with a delay before the decay
        plot_speed_max_1000 -= 1;
        uint32_t new_max = (plot_speed_max_1000 + 500) / 1000;
        if (new_max < plot_speed_max) {
            // delay has ended, actually decay
            plot_speed_max = new_max;
        }
    }
    #endif
}

#ifdef ENABLE_PLOT_RPM
uint16_t plot_speed_to_pixels(int32_t x)
{
    return gui_screenHeight - map_rounded(x, 0, plot_speed_max, 0, gui_screenHeight - 1, true);
}
#endif

void plot_draw()
{
    int i;
    int h, h2;
    int x, x2;
    for (i = 0, h = history_idx; i < PLOT_LENGTH - 1; i++, h--) {
        if (h < 0) {
            h = PLOT_LENGTH - 1;
        }
        h2 = h - 1;
        if (h2 < 0) {
            h2 = PLOT_LENGTH - 1;
        }
        x = PLOT_LENGTH - i - 1;
        plot_data_t* ptr1 = &(history[h ]);
        plot_data_t* ptr2 = &(history[h2]);
        sprite.drawLine(x, ptr1->current_min, x2, ptr2->current_min, TFT_RED);
        sprite.drawLine(x, ptr1->current_avg, x2, ptr2->current_avg, TFT_RED);
        sprite.drawLine(x, ptr1->current_max, x2, ptr2->current_max, TFT_RED);
        sprite.drawLine(x, ptr1->voltage_max, x2, ptr2->voltage_max, TFT_GREEN);
        sprite.drawLine(x, ptr1->voltage_avg, x2, ptr2->voltage_avg, TFT_GREEN);
        sprite.drawLine(x, ptr1->voltage_min, x2, ptr2->voltage_min, TFT_GREEN);
        #if ENABLE_PLOT_RPM
        sprite.drawLine(x, plot_speed_to_pixels(ptr1->speed_min), x2, plot_speed_to_pixels(ptr2->speed_min), TFT_YELLOW);
        sprite.drawLine(x, plot_speed_to_pixels(ptr1->speed_avg), x2, plot_speed_to_pixels(ptr2->speed_avg), TFT_YELLOW);
        sprite.drawLine(x, plot_speed_to_pixels(ptr1->speed_max), x2, plot_speed_to_pixels(ptr2->speed_max), TFT_YELLOW);
        #endif
        if (ptr1->bec_fault) {
            sprite.drawLine(x, gui_screenHeight - 1, x, gui_screenHeight / 2, TFT_ORANGE);
        }
    }
}

#include <Arduino.h>

int32_t div_rounded(const int n, const int d)
{
    return ((n < 0) ^ (d < 0)) ? ((n - (d / 2)) / d) : ((n + (d / 2)) / d);
}

int32_t map_rounded(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max, bool limit)
{
    int32_t a = x - in_min;
    int32_t b = out_max - out_min;
    int32_t c = in_max - in_min;
    int32_t d = a * b;
    int32_t y = div_rounded(d, c);
    y += out_min;
    if (limit)
    {
        if (out_max > out_min)
        {
            if (y > out_max) {
                return out_max;
            }
            else if (y < out_min) {
                return out_min;
            }
        }
        else
        {
            if (y < out_max) {
                return out_max;
            }
            else if (y > out_min) {
                return out_min;
            }
        }
    }
    return y;
}
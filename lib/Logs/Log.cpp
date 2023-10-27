#include "Log.hpp"

void service_log(const char* TAG, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Serial.printf("%s: %s\n",TAG,fmt);
    va_end(args);
}
#ifndef HELPERS_H_
#define HELPERS_H_

#include <stdio.h>
#include "pico/stdlib.h"
#include "helpers.h"
#include <string.h>
#include <stdarg.h>

#define UART_ID uart1

void printy(const char * format, ...);
#endif
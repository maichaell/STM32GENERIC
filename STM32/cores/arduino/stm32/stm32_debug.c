/*
  Copyright (c) 2017 Daniel Fekete

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "stm32_debug.h"

#include "Arduino.h"

#pragma GCC diagnostic ignored "-Wformat"

char _stderr_buf[80];           //for stderr outbuf add by huaweiwx@sina.com  2017.12.8

//debug_if add by huaweiwx@sina.com  2017.12.8
void debug(const char *format, ...) {
    setbuf(stderr,_stderr_buf);  //set stderr outbuf add by huaweiwx@sina.com  2017.12.8
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "");
}

//debug_if add by huaweiwx@sina.com  2017.12.8
void debug_if(int condition, const char *format, ...) {	
    setbuf(stderr,_stderr_buf);
    if (condition) {
       va_list args;
       va_start(args, format);
       vfprintf(stderr, format, args);
       va_end(args);
       fprintf(stderr, "");
    }
}

void print_log(const char *level, const char *format, const char *file, const int line, ...) {
    setbuf(stderr,_stderr_buf);  //set stderr outbuf add by huaweiwx@sina.com  2017.12.8
    uint32_t m = micros();

    uint32_t seconds = m / 1000000;
    uint32_t fractions = m % 1000000;

    fprintf(stderr, "[%2u.%-6u]%10s %3d %s:", seconds, fractions, file, line, level);

    va_list argList;

    va_start(argList, line);
    vfprintf(stderr, format, argList);
    va_end(argList);
    fprintf(stderr, "\n");
}

char *stm32PortPinName(GPIO_TypeDef *port, uint32_t pinMask) {
    for(size_t i=0; i<sizeof(variant_pin_list) / sizeof(variant_pin_list[0]); i++) {
        if (variant_pin_list[i].port == port && variant_pin_list[i].pin_mask == pinMask) {
            return stm32PinName(i);
        }
    }
//    return (char*)"Unknown pin";
    return NULL;
}

char *stm32PinName(uint8_t pin) {
    if (pin >= NUM_DIGITAL_PINS) {
//        return (char*)"Unknown";
        return NULL;
    }
    static char ret[10];
    int index = 0;

    if (variant_pin_list[0].port != GPIOA || variant_pin_list[0].pin_mask != GPIO_PIN_0) {
        if (pin < 10) {
            ret[index++] = '0' + pin;
        } else {
            ret[index++] = '0' + pin / 10;
            ret[index++] = '0' + pin % 10;
        }
        ret[index++] = ' ';
        ret[index++] = '(';
    }

    ret[index++] = 'P';

    stm32_port_pin_type port_pin = variant_pin_list[pin];

    ret[index++] = 'A' + ((uint32_t)port_pin.port - (uint32_t)GPIOA) / ((uint32_t)GPIOB - (uint32_t)GPIOA);
    int num = __builtin_ffs(port_pin.pin_mask) - 1;
    if (num < 10) {
        ret[index++] = '0' + num;
    } else {
        ret[index++] = '0' + num / 10;
        ret[index++] = '0' + num % 10;
    }

    if (variant_pin_list[0].port != GPIOA || variant_pin_list[0].pin_mask != GPIO_PIN_0) {
        ret[index++] = ')';
    }

    ret[index] = 0;

    return ret;

}

//assert_failed() used by stm32_assert.h. add by huaweiwx@sina.com  2017.12.8
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
	debug("\nassert_failed in file: %s line:%d\n",(char *)file,line);
	while(1)
		yield();
};
#endif

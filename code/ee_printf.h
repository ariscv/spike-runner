#ifndef _WHISPER_PRINTF_H_
#define _WHISPER_PRINTF_H_

#include <stdarg.h>
#define ee_printf(fmt,...) whisperPrintf(fmt, ##__VA_ARGS__)
#define ee_vprintf(fmt,ap) whisperPrintfImpl(fmt,ap)

int whisperPrintfImpl(const char *format, va_list ap);
int whisperPrintf(const char *format, ...);
int whisperPutc(char c);

#endif // !_WHISPER_PRINTF_H_
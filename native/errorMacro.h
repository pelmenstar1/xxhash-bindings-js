#ifndef _ERROR_MACRO
#define _ERROR_MACRO

#include <cstdint>

#define FATAL_ERROR(msg) FatalError(msg, __FILE__, __LINE__)

void FatalError(const char* message, const char* fileName, uint32_t lineNumber);

#endif
#include "errorMacro.h"

void FatalError(const char* message, const char* fileName, uint32_t lineNumber) {
  std::cerr << "Fatal error: " << message << std::endl;
  std::cerr << "    At: " << fileName << ':' << lineNumber << std::endl;

  exit(-1);
}
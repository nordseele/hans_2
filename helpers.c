#include "helpers.h"

// replaces the regular printf function
void printy(const char* format, ...) {
  va_list args;
  va_start(args, format);
  
  char message[256]; // Allocate space for the output string
  
  vsnprintf(message, sizeof(message), format, args); // Format the string with the given arguments
  
  va_end(args);	
  strcat(message, "\r\n"); // Add carriage return and newline

  uart_puts(UART_ID, message); // Print the output string to the UART
}

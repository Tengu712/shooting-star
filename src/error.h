#pragma once

typedef int warn_t;
#define WARNING 0
#define SUCCESS 1

warn_t init_logger(void);
void error(const char *msg);
warn_t warning(const char *msg);
void log_info(const char *msg);
void log_debug(const char *format, ...);

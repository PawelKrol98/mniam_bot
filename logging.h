#ifndef LOGGING_H_
#define LOGGING_H_
#include <stdio.h>

#define LOG_LEVEL 3

#if LOG_LEVEL >= 1 
#define LOG_ERR(...) \
printf("[ERR] "); \
printf(__VA_ARGS__); \
printf("\n");
#else
#define LOG_ERR(...)
#endif

#if LOG_LEVEL >= 2 
#define LOG_WRN(...) \
printf("[WRN] "); \
printf(__VA_ARGS__); \
printf("\n");
#else
#define LOG_WRN(...)
#endif

#if LOG_LEVEL >= 3 
#define LOG_INF(...) \
printf("[INF] "); \
printf(__VA_ARGS__); \
printf("\n");
#else
#define LOG_INF(...)
#endif

#if LOG_LEVEL >= 4 
#define LOG_DBG(...) \
printf("[DBG] "); \
printf(__VA_ARGS__); \
printf("\n");
#else
#define LOG_DBG(...)
#endif

#if LOG_LEVEL >= 5
#define LOG_DBG2(...) \
printf("[DBG2] "); \
printf(__VA_ARGS__); \
printf("\n");
#else
#define LOG_DBG2(...)
#endif

#endif /* LOGGING_H_ */

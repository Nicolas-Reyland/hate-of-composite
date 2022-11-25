#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

#define DEFAULT_LOG_LEVEL 2

extern int LOG_LEVEL;

#define LOG_MESSAGE(LogLevelThreshold, Prefix, Format, ...)                    \
    {                                                                          \
        if (LOG_LEVEL > (LogLevelThreshold))                                   \
        {                                                                      \
            fprintf(stderr,                                                    \
                    Prefix " (\033[35m%s\033[39m:\033[34m%d\033[39m) "         \
                           "[\033[93m%s\033[39m]: " Format "\n",               \
                    __FILE__, __LINE__, __func__, ##__VA_ARGS__);              \
        }                                                                      \
    }

#define LOG_ERROR(Format, ...)                                                 \
    LOG_MESSAGE(0, "\033[91m{ERROR}\033[39m", Format, ##__VA_ARGS__)

#define LOG_WARN(Format, ...)                                                  \
    LOG_MESSAGE(1, "\033[33m{WARN}\033[39m", Format, ##__VA_ARGS__)

#define LOG_INFO(Format, ...)                                                  \
    LOG_MESSAGE(2, "\033[32m{INFO}\033[39m", Format, ##__VA_ARGS__)

#define LOG_DEBUG(Format, ...)                                                 \
    LOG_MESSAGE(3, "\033[94m{DEBUG}\033[39m", Format, ##__VA_ARGS__)

#endif /* !LOGGING_H */

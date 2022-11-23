#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

#define DEFAULT_LOG_LEVEL 2

extern int LOG_LEVEL;

#define LOG_ERROR(Format, ...)                                                 \
    {                                                                          \
        if (LOG_LEVEL > 0)                                                     \
        {                                                                      \
            fprintf(stderr, "\033[91m{ERROR}\033[39m %s: " Format "\n",        \
                    __func__, ##__VA_ARGS__);                                  \
        }                                                                      \
    }

#define LOG_WARN(Format, ...)                                                  \
    {                                                                          \
        if (LOG_LEVEL > 1)                                                     \
        {                                                                      \
            fprintf(stderr, "\033[33m{WARN}\033[39m %s: " Format "\n",         \
                    __func__, ##__VA_ARGS__);                                  \
        }                                                                      \
    }

#define LOG_INFO(Format, ...)                                                  \
    {                                                                          \
        if (LOG_LEVEL > 2)                                                     \
        {                                                                      \
            fprintf(stderr, "\033[32m{INFO}\033[39m %s: " Format "\n",         \
                    __func__, ##__VA_ARGS__);                                  \
        }                                                                      \
    }

#define LOG_DEBUG(Format, ...)                                                 \
    {                                                                          \
        if (LOG_LEVEL > 3)                                                     \
        {                                                                      \
            fprintf(stderr, "\033[94m{DEBUG}\033[39m %s: " Format "\n",        \
                    __func__, ##__VA_ARGS__);                                  \
        }                                                                      \
    }

#endif /* !LOGGING_H */

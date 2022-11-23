#include "logging.h"

/*
 * 0: quiet (not even error messages)
 * 1: no warnings
 * 2: default (error & warning messages)
 * 3: default + info messages
 * 4: default + info & debug messages
 */
int LOG_LEVEL = DEFAULT_LOG_LEVEL;

#ifndef CLI_H_
#define CLI_H_

#include <stdbool.h>

enum Jdwm_Flags {
        FLAG_CUSTOM_CONFIG,
        FLAG_CUSTOM_LOG_LEVEL,
        FLAG_DISABLE_DEFAULT_LOGGER,
        FLAG_NO_JDWMRC,
        FLAGS_COUNT
};

extern char **build_restart_args( void );
extern void jdwm_parse_args( int argc, char **argv );

extern bool g_cli_flags[ FLAGS_COUNT ];

#endif /* CLI_H_ */

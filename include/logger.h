// Heavy credit goes to rxi (https://github.com/rxi) and (https://github.com/JeepWay) whose c loggers I HEAVILY borrowed/copied from to create this logger

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_OUTPUT_HANDLERS 32
#define DATE_TIME_FORMAT "%H:%M:%S"
#define NO_COLOR_LOG_FORMAT "%s %-5s [%s::%s::%d]: "
#define COLOR_LOG_FORMAT "%s %s%-5s\x1b[0m \x1b[90m[%s::%s::%d]:\x1b[0m "

// Default logger state
#define DEFAULT_LOGGER_LEVEL LOG_DEBUG
#define DEFAULT_LOGGER_NAME "Default Logger"
#define DEFAULT_LOG_STREAM stdout
#define DEFAULT_LOG_STREAM_IS_QUIET false
#define DEFAULT_LOG_STREAM_USE_COLOR true

enum Log_Level {
        LOG_TRACE,
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        LOG_FATAL,
};

typedef struct Output_Handler {
        const char *name;
        void *file_pointer;
        unsigned int level;
        bool quiet;
        bool color;
} Output_Handler_t;

typedef struct Log_Event {
        unsigned int log_level;
        const char *filename;
        const char *function_name;
        int line;
        const char *format;
        void *user_data;
        struct tm *time;
        va_list arguments;
} Log_Event_t;

typedef struct Logger {
        unsigned int count;
        Output_Handler_t output_handlers[ MAX_OUTPUT_HANDLERS ];
} Logger_t;

int logger_add_file_handler( const char *handler_name, const char *filepath, const char *filemode, unsigned int log_level, bool is_quiet, bool use_color );
int logger_add_stream_handler( const char *handler_name, FILE *file_pointer, unsigned int log_level, bool is_quiet, bool use_color );
int logger_set_log_level( const char *handler_name, unsigned int log_level );
int logger_set_quiet( const char *handler_name, bool quiet );
void log_message( unsigned int log_level, const char *filename, const char *function_name, int line, const char *format, ... );

#ifdef _DEBUG
#define log_trace( ... ) log_message( LOG_TRACE, __FILE__, __func__, __LINE__, __VA_ARGS__ )
#else
#define log_trace( ... )
#endif

#define log_debug( ... ) log_message( LOG_DEBUG, __FILE__, __func__, __LINE__, __VA_ARGS__ )
#define log_info( ... ) log_message( LOG_INFO, __FILE__, __func__, __LINE__, __VA_ARGS__ )
#define log_warn( ... ) log_message( LOG_WARN, __FILE__, __func__, __LINE__, __VA_ARGS__ )
#define log_error( ... ) log_message( LOG_ERROR, __FILE__, __func__, __LINE__, __VA_ARGS__ )
#define log_fatal( ... ) { log_message( LOG_FATAL, __FILE__, __func__, __LINE__, __VA_ARGS__ ); exit( 1 ); }

extern const char *LOG_LEVEL_STRINGS[ ];
extern const char *LOG_LEVEL_COLORS[ ];

#endif /* LOGGER_H_ */
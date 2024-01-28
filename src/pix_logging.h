#ifndef P_LOGGING_H
#define P_LOGGING_H 1

// NOTE: Update this to VERBOSE(A) A in local program to create verbose options
#ifndef VERBOSE
#define VERBOSE(A)
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

// NOTE: Update this in the local program to set the degree of logs being set.
#ifndef P_LOG_LEVEL
#define P_LOG_LEVEL 7
#endif

// NOTE: Update this in the local program to unset the values
#ifndef P_LOG_COLOUR
#define P_LOG_COLOUR 1
#endif

// NOTE: Update this in the local program to choose where log messages go out
// to.
#ifndef P_LOG_OUT
#define P_LOG_OUT stderr
#endif

// Same as syslog
#define EMERG 0
#define ALERT 1
#define CRIT 2
#define ERR 3
#define WARN 4
#define SUCCESS 5
#define INFO 6
#define DBG 7

// Better colour definition
#define NORM "\x1b[0m"
#define BLACK "\x1b[0;30m"
#define L_BLACK "\x1b[1;30m"
#define RED "\x1b[0;31m"
#define L_RED "\x1b[1;31m"
#define GREEN "\x1b[0;32m"
#define L_GREEN "\x1b[1;32m"
#define BROWN "\x1b[0;33m"
#define YELLOW "\x1b[1;33m"
#define BLUE "\x1b[0;34m"
#define L_BLUE "\x1b[1;34m"
#define PURPLE "\x1b[0;35m"
#define L_PURPLE "\x1b[1;35m"
#define CYAN "\x1b[0;36m"
#define L_CYAN "\x1b[1;36m"
#define GREY "\x1b[0;37m"
#define WHITE "\x1b[1;37m"

// Colour utils
#define BOLD "\x1b[1m"
#define UNDERLINE "\x1b[4m"
#define BLINK "\x1b[5m"
#define REVERSE "\x1b[7m"
#define HIDE "\x1b[8m"
#define CLEAR "\x1b[2J"
/* #define CLRLINE "\r\e[K" // or "\e[1K\r" */

// New hack to get the filename I found :D
#define __FILENAME__                                                           \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// Could pull out errno here with this found in syslog:
#define err_to_str() (errno == 0 ? "" : strerror(errno))

// Log functions
#define p_emrg(msg, ...)                                                       \
  do {                                                                         \
    fprintf(P_LOG_OUT,                                                         \
            RED "[!!!]: "                                                      \
                "%s (%s:%d) " NORM msg YELLOW " errno: %s\n" NORM,             \
            __func__, __FILE__, __LINE__, ##__VA_ARGS__, err_to_str());        \
  } while (0)

#define p_alert(msg, ...)                                                      \
  do {                                                                         \
    fprintf(P_LOG_OUT, PURPLE "[A!]: " NORM msg "\n", ##__VA_ARGS__);          \
  } while (0)

#define p_crit(msg, ...)                                                       \
  do {                                                                         \
    fprintf(P_LOG_OUT,                                                         \
            RED "[!!]: "                                                       \
                "%s (%s:%d) " NORM msg YELLOW " errno: %s\n" NORM,             \
            __func__, __FILE__, __LINE__, ##__VA_ARGS__, err_to_str());        \
  } while (0)

#define p_err(msg, ...)                                                        \
  do {                                                                         \
    fprintf(P_LOG_OUT, RED "[!]: " NORM msg "\n", ##__VA_ARGS__);              \
  } while (0)

#define p_warn(msg, ...)                                                       \
  do {                                                                         \
    fprintf(P_LOG_OUT, BLUE "[W]: " NORM msg "\n", ##__VA_ARGS__);             \
  } while (0)

#define p_success(msg, ...)                                                    \
  do {                                                                         \
    fprintf(P_LOG_OUT, GREEN "[S]: " NORM msg "\n", ##__VA_ARGS__);            \
  } while (0)

#define p_info(msg, ...)                                                       \
  do {                                                                         \
    fprintf(P_LOG_OUT, CYAN "[+]: " NORM msg "\n", ##__VA_ARGS__);             \
  } while (0)

// The ability to pull out lines and func can be extended to others if required
#define p_dbg(msg, ...)                                                        \
  do {                                                                         \
    fprintf(P_LOG_OUT, GREY "[D]: " NORM msg "\n", ##__VA_ARGS__);             \
  } while (0)

#define p_dbgv(msg, ...)                                                       \
  do {                                                                         \
    fprintf(P_LOG_OUT, GREY "[D]: %s (%s:%d) " NORM msg "\n", __func__,        \
            __FILE__, __LINE__, ##__VA_ARGS__);                                \
  } while (0)

#define p_log(msg, ...)                                                        \
  do {                                                                         \
    fprintf(P_LOG_OUT, "[>]: " msg "\n", ##__VA_ARGS__);                       \
  } while (0)

// Log level controls -- Syslog style
//
#ifndef DEBUG
#undef p_dbg
#define p_dbg(msg, ...)                                                        \
  do {                                                                         \
  } while (0)
#undef p_dbgv
#define p_dbgv(msg, ...)                                                       \
  do {                                                                         \
  } while (0)
#endif

#if P_LOG_LEVEL < INFO
#undef p_info
#define p_dbg(msg, ...)                                                        \
  do {                                                                         \
  } while (0)
#endif

#if P_LOG_LEVEL < SUCCESS
#undef p_p_success
#define p_success(msg, ...)                                                    \
  do {                                                                         \
  } while (0)
#endif

#if P_LOG_LEVEL < WARN
#undef p_warn
#define p_warn(msg, ...)                                                       \
  do {                                                                         \
  } while (0)
#endif

#if P_LOG_LEVEL < ERR
#undef p_err
#define p_err(msg, ...)                                                        \
  do {                                                                         \
  } while (0)
#endif

#if P_LOG_LEVEL < ALERT
#undef p_alert
#define p_alert(msg, ...)                                                      \
  do {                                                                         \
  } while (0)
#endif

#if P_LOG_LEVEL < EMERG
#undef p_emrg
#define p_emrg(msg, ...)                                                       \
  do {                                                                         \
  } while (0)
#endif

// Turn off colors.
#if P_LOG_COLOUR < 1
#undef NORM
#define NORM
#undef RED
#define RED
#undef PURPLE
#define PURPLE
#undef YELLOW
#define YELLOW
#undef BROWN
#define BROWN
#undef GREEN
#define GREEN
#undef CYAN
#define CYAN
#undef BLUE
#define BLUE
#undef GREY
#define GREY
#endif

#endif

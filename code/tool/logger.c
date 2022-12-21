#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "logger.h"
#include "stringhelp.h"

typedef void (*logfct)(const enum log_level ll, const char * format, va_list argp);

static struct logger_state
{
  size_t        level[LL_COUNT];
  logfct        fct;
} log;

const char * level_txt[] = {
   "NONE",
   "CRIT ",
   "ERROR",
   "WARN ",
   "INFO ",
   "EVENT",
   "DEBUG",
   "DBGMR",
   "DBGMX"
};

const char * level_fulltxt[]  = {
    "NONE",
    "CRITICAL",
    "ERROR",
    "WARNING",
    "INFO",
    "EVENT",
    "DEBUG",
    "DEBUG_MORE",
    "DEBUG_MAX"
 };

const char * facility_txt [] = {
  "stdout" ,
  "user"   ,
  "local0" ,
  "local1" ,
  "local2" ,
  "local3" ,
  "local4" ,
  "local5" ,
  "local6" ,
  "local7"
};

static void log_stdout_stderr(const enum log_level ll, const char * format, va_list ap)
{
  static char tmp[MAX_LOG_LEN];

  FILE * fd;
  time_t ts = time(NULL);
  char * tim = asctime(localtime(&ts));
  size_t len = strlen(tim);
  if (len)
    tim[len - 1] = '\0';

  vsprintf(tmp, format, ap);

  tmp[sizeof(tmp) - 1] = '\0';

  switch (ll)
  {
    case LL_CRITICAL:
    case LL_ERROR   :
    case LL_WARN    : fd = stderr; break;
    default         : fd = stdout; break;
  }
  fprintf (fd, "[%s][%s] %s\n", tim, level_txt[ll], tmp);
}



const int ll_translation[LL_COUNT] =
{
  LOG_CRIT,    /*  LL_CRITICAL    */
  LOG_ERR,     /*  LL_ERROR       */
  LOG_WARNING, /*  LL_WARN        */
  LOG_NOTICE,  /*  LL_INFO        */
  LOG_INFO,    /*  LL_EVENT       */
  LOG_DEBUG,   /*  LL_DEBUG       */
  LOG_DEBUG,   /*  LL_DEBUG_MORE  */
  LOG_DEBUG    /*  LL_DEBUG_MAX   */
};

static void log_syslog(const enum log_level ll, const char * format, va_list ap)
{
  vsyslog(ll_translation[ll], format, ap);
}

const int lf_translation[LF_COUNT] =
{
  LOG_USER	  , /*  LF_USER     random user-level messages */
  LOG_LOCAL0  , /*  LF_LOCAL0   reserved for local use */
  LOG_LOCAL1  , /*  LF_LOCAL1   reserved for local use */
  LOG_LOCAL2  , /*  LF_LOCAL2   reserved for local use */
  LOG_LOCAL3  , /*  LF_LOCAL3   reserved for local use */
  LOG_LOCAL4  , /*  LF_LOCAL4   reserved for local use */
  LOG_LOCAL5  , /*  LF_LOCAL5   reserved for local use */
  LOG_LOCAL6  , /*  LF_LOCAL6   reserved for local use */
  LOG_LOCAL7  , /*  LF_LOCAL7   reserved for local use */
};


void log_init(const char * ident, enum log_facility facility)
{
  memset(&log,0,sizeof(log));

  if (facility > LF_STDOUT && facility < LF_COUNT && ident)
  {
    openlog(ident, 0, lf_translation[facility - 1]);
    log.fct = log_syslog;
  }
  else
  {
    log.fct = log_stdout_stderr;
  }
}

void log_set_level(enum log_level ll, size_t active)
{
  if (active)
  {
    log.level[ll] = active;
    for (--ll; ll > LL_NONE; ll--)
      log.level[ll] = active;
  }
  else log.level[ll] = FALSE;
}

int log_get_level(enum log_level ll)
{
  return log.level[ll];
}

const char * log_get_level_name(enum log_level ll)
{
  if (ll > 0 && ll < ARRLEN(level_fulltxt))
    return level_fulltxt[ll];
  return NULL;
}

enum log_level log_get_level_no(const char * level)
{
  for (int i = 0; i < ARRLEN(level_fulltxt); i++)
  {
    if (stricmp(level_fulltxt[i], level) == 0)
      return (enum log_level) i;
  }
  return LL_NONE;
}

enum log_facility log_get_facility(const char * facility)
{
  for (int i = 0; i < ARRLEN(facility_txt); i++)
  {
    if (stricmp(facility_txt[i], facility) == 0)
      return (enum log_facility) i;
  }
  return LF_COUNT;
}

const char * log_get_facility_name(enum log_facility lf)
{
  if  (lf > 0 && lf < ARRLEN(facility_txt))
    return facility_txt[lf];
  return NULL;
}


void log_push(const enum log_level ll, const char * format, ...)
{
  if (ll >= 0 && ll < LL_COUNT && log.level[ll] && log.fct)
  {
    va_list ap;
    va_start(ap, format);
    log.fct(ll, format, ap);
    va_end(ap);
  }
}

#ifndef _DEBUG_H
#define _DEBUG_H
// avp 2016 simple debug for C
#include <stdio.h>
#include <stdlib.h>

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif

static int debug_exit_level  = DEBUG_LEVEL;
static FILE *debug_out = 0;
static char *env_level = 0;
  

static int
debug_message (int level, const char *expr, const char *msg,
	       const char *file, int line, const char *func)
{
  if (!env_level) {
    if ((env_level = getenv("DEBUG_LEVEL")))
      debug_exit_level = atoi(env_level);
    else
      env_level = (char *)"";
  }
  if (!debug_out)
    debug_out = stderr;
  
  int need_exit = level < debug_exit_level;

  if (msg || need_exit)
    fprintf(debug_out, "DEBUG: %s line %d (%s) %s [%s]\n",
	    file, line, func, expr, msg);
  if (need_exit)
    abort();

  return level;
}

#define DEBUG(l,e,t) \
  ((e) ? \
   0	 \
   : debug_message(l, __STRING(e), t, __FILE__, __LINE__, __FUNCTION__))

#endif
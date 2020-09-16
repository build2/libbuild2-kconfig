#include "confapi.h"

#define main(x,y) unused_main(x,y)
#include "conf.c"

// NOTE: if we were to support other modes, then we may need to (re)initialize
//       more state.
//
static void init (enum input_mode m)
{
  input_mode = m;
  indent = 1;
  tty_stdio = isatty (0) && isatty (1);
  rootEntry = NULL;
}

static void conf_ask_impl ()
{
  do {
    conf_cnt = 0;
    check_conf (&rootmenu);
  } while (conf_cnt);
}

void conf_ask ()
{
  init (oldconfig);
  conf_ask_impl ();
}

void conf_reask ()
{
  init (oldaskconfig);
  rootEntry = &rootmenu;
  conf(&rootmenu);
  input_mode = oldconfig;
  conf_ask_impl ();
}

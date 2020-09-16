#ifndef GETOPT_H
#define GETOPT_H

/* Dummy getopt implementation for conf.c on Windows. */

static int optind;
static int optopt;
static int opterr;
static char *optarg;

struct option
{
  const char *name;
  int         has_arg;
  int        *flag;
  int         val;
};

enum
{
  no_argument = 0,
  required_argument,
  optional_argument
};

inline int getopt_long(int nargc, char * const *nargv, const char *options,
                       const struct option *long_options, int *idx)
{
  return -1;
}

#endif /* GETOPT_H */

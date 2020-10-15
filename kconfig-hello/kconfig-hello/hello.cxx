#include <string>

#include <kconfig-hello/config.hxx>

#if IO
#  include <kconfig-hello/io.hxx>
#endif

using namespace std;

int
main (int argc, char* argv[])
{
  // Get name and greeting depending on the configuration.
  //
  // Besides showing the use of Kconfig options in our code, this also
  // highlights how hairy things can get.
  //
  string name, greet;

  if (argc > 1)
    name = argv[1];

#if !FANCY
  greet = "Hello,";
#else

#  if NAME_FALLBACK
  if (name.empty ())
  {
#    if NAME_DEF

    name = "World";

#    elif NAME_ASK

    for (int i (0); name.empty () && i != NAME_ASK_MAX; ++i)
      name = ask ("enter name: ");

#    else
#      error unhandled name fallback
#    endif
  }
#  endif // NAME_FALLBACK

  greet = GREETING_TEXT;

#  if GREETING_BUILTIN
  greet += ',';
#  endif

#endif // FANCY

  if (name.empty ())
  {
#if IO
    print_err ("error: missing name\n");
#endif
    return 1;
  }

  // Format the greeting sentence.
  //
  greet += ' ';
  greet += name;
  greet += PUNCT;
  greet += '\n';

#if IO
  print_out (greet);
#endif
}

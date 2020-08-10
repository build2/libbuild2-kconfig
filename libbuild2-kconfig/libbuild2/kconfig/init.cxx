#include <libbuild2/kconfig/init.hxx>

#include <libbuild2/diagnostics.hxx>

using namespace std;

namespace build2
{
  namespace kconfig
  {
    bool
    init (scope&,
          scope&,
          const location& l,
          bool,
          bool,
          module_init_extra&)
    {
      info (l) << "module kconfig initialized";
      return true;
    }

    static const module_functions mod_functions[] =
    {
      {"kconfig", nullptr, init},
      {nullptr,   nullptr, nullptr}
    };

    const module_functions*
    build2_kconfig_load ()
    {
      return mod_functions;
    }
  }
}

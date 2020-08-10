#pragma once

#include <libbuild2/types.hxx>
#include <libbuild2/utility.hxx>

#include <libbuild2/module.hxx>

#include <libbuild2/kconfig/export.hxx>

namespace build2
{
  namespace kconfig
  {
    extern "C" LIBBUILD2_KCONFIG_SYMEXPORT const module_functions*
    build2_kconfig_load ();
  }
}

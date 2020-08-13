#include <libbuild2/kconfig/init.hxx>

#include <libbuild2/file.hxx>
#include <libbuild2/scope.hxx>
#include <libbuild2/filesystem.hxx>
#include <libbuild2/diagnostics.hxx>

#include <libbuild2/kconfig/lkc.h>

using namespace std;

namespace build2
{
  namespace kconfig
  {
    bool
    init (scope& rs,
          scope& bs,
          const location& l,
          bool,
          bool,
          module_init_extra&)
    {
      tracer trace ("kconfig::init");
      l5 ([&]{trace << "for " << rs;});

      // We only support root loading.
      //
      // @@ TODO: and only in root.build
      //
      if (rs != bs)
        fail (l) << "kconfig module must be loaded in project root";

      context& ctx (rs.ctx);

      path df (rs.src_path () / rs.root_extra->build_dir / "Kconfig");
      path vf (rs.out_path () / rs.root_extra->build_dir / "config.kconfig");

      if (!exists (df))
        fail (l) << df << " does not exist";

      // (Re)configure if we are configuring.
      //
      // @@ TODO: support customzing mode/method via config.kconfig?
      //
      // @@ COMP: will need to aggregate/disaggregate both files?
      //
      if (ctx.current_mif->id == configure_id)
      {
        //@@ We should only save the config to final place in hook during
        //   execute. Also, kconfig-conf moves old config to *.old.

        // What should be the default configuration mode? If the config file
        // doesn't exist, then Q&A (--oldaskconfig). Otherwise, update the
        // existing (--oldconfig).
        //

        //@@ Before importing and running the tool, we could check ourselves
        //   whether existing config is good.
        //
        //   We could also fallback to built-in implementation (for some
        //   basic modes) if unable to import.
        //
        pair<const exe*, import_kind> ir (
          import_direct<exe> (
            rs,
            name ("kconfig-conf", dir_path (), "exe", "kconfig-conf"),
            true      /* phase2 */,
            false     /* optional */,
            false     /* metadata */,
            l,
            "module load"));

        //@@ If we were to support normal import, we would have to update the
        //   target. Maybe one day.
        //
        if (ir.second != import_kind::adhoc)
          fail (l) << "project import of kconfig-conf not supported" <<
            info << "use config.kconfig_conf to specify executable path";

        const process_path& pp (ir.first->process_path ());
        cstrings args {pp.recall_string ()};

        args.push_back ("-s");
        args.push_back (exists (vf) ? "--oldconfig" : "--oldaskconfig");
        args.push_back (df.string ().c_str ());
        args.push_back (nullptr);

        strings vars;
        vars.push_back ("KCONFIG_CONFIG=" + vf.string ()); // Keep it first.

        process_env env (pp, vars);

        if (verb >= 3)
          print_process (env, args);
        else if (verb >= 2)
        {
          diag_record dr (text);
          dr << vars[0] << ' '; print_process (dr, args);
        }
        else if (verb)
          text << "kconfig " << vf;

        run (env, args);
      }

      // @@ kconfig issues:
      //
      // - Diagnostics is all over the place, exit(1) on error.
      //
      auto conf_g (make_guard ([] () {conf_free ();}));

      // Disable extra diagnostics.
      //
      conf_set_message_callback (nullptr);

      // Load the configuration definition (Kconfig).
      //
      conf_parse (df.string ().c_str ());

      // Load the configuration values.
      //
      if (!exists (vf))
        fail (l) << vf << " does not exist" <<
          info << "consider configuring " << rs.out_path ();

      if (conf_read (vf.string ().c_str ()) != 0)
        fail (l) << "unable to load " << vf;

      if (conf_get_changed ())
        fail (l) << "kconfig configuration definition/value have changed" <<
          info << "reconfigure to synchronize" <<
          info << "configuration definition: " << df <<
          info << "configuration values: " << vf;

      // Set kconfig symbols as kconfig.* variables.
      //
      {
        auto& vp (rs.var_pool ());

        size_t i;
        string n; // Reuse the symbol name buffer.
        const symbol* s;
        for_all_symbols (i, s)
        {
          // See kconfig-dump for semantics of these tests.
          //
          if (s->name == nullptr   ||
              s->type == S_UNKNOWN ||
              (s->flags & SYMBOL_CONST) != 0)
            continue;

          assert (s->flags & SYMBOL_VALID);

          // Process the name.
          //
          string var_n ("kconfig.");
          bool var_q;
          {
            n = s->name;
            lcase (n);

            // See if it's project-qualified.
            //
            size_t p (n.find ("__"));
            var_q = (p != string::npos && p + 2 != n.size ());
            if (var_q)
            {
              //@@ TODO: verify matches project name (icase).

              var_n.append (n, 0, p);
              var_n += '.';
              var_n.append (n, p + 2, string::npos);
            }
            else
              var_n += n;
          }

          // Enter variable.
          //
          // @@ The kconfig module could define the tristate type if there use
          //    for it.
          //
          const value_type* var_t (nullptr);
          if (var_q)
          {
            switch (s->type)
            {
            case S_BOOLEAN:  var_t = &value_traits<bool>::value_type;     break;
            case S_TRISTATE: var_t = /* untyped */ nullptr;               break;
            case S_INT:      var_t = &value_traits<int64_t>::value_type;  break;
            case S_HEX:      var_t = &value_traits<uint64_t>::value_type; break;
            case S_STRING:   var_t = &value_traits<string>::value_type;   break;
            case S_UNKNOWN:  assert (false);
            }
          }

          const variable& var (vp.insert (move (var_n),
                                          var_t,
                                          true /* overridable */,
                                          (var_q
                                           ? variable_visibility::global
                                           : variable_visibility::project)));

          // Assign the value.
          //
          const symbol_value& v (s->curr);

          value& val (rs.assign (var));
          switch (s->type)
          {
          case S_BOOLEAN:
            {
              val = (v.tri == yes);
              break;
            }
          case S_TRISTATE:
            {
              val = names {name (v.tri == no  ? "false" :
                                 v.tri == mod ? "module" : "true")};
              break;
            }
          case S_INT:
          case S_HEX:
          case S_STRING:
            {
              // These are all represented as strings.
              //
              // Note that in case of S_STRING it seems the string may contain
              // escapes but what we get is unescaped.
              //
              // See kconfig-dump for details on the NULL semantics.
              //
              // @@ Why doesn't null apply to bool and tristate?
              //
              bool null (!(s->flags & SYMBOL_WRITE) &&
                         !(s->flags & SYMBOL_NO_WRITE));
              if (null)
              {
                val = nullptr;
                break;
              }

              string sv (static_cast<const char*> (v.val));
              switch (s->type)
              {
              case S_INT:
                {
                  try
                  {
                    // May throw invalid_argument or out_of_range.
                    //
                    size_t i;
                    val = stoll (sv, &i);

                    if (i == sv.size ())
                      break;

                    // Fall through.
                  }
                  catch (const std::exception&)
                  {
                    // Fall through.
                  }

                  fail (l) << "invalid kconfig int value '" << sv << "'"
                           << endf;
                }
              case S_HEX:
                {
                  try
                  {
                    // May throw invalid_argument or out_of_range.
                    //
                    size_t i;
                    val = stoull (sv, &i, 16);

                    if (i == sv.size ())
                      break;

                    // Fall through.
                  }
                  catch (const std::exception&)
                  {
                    // Fall through.
                  }

                  fail (l) << "invalid kconfig hex value '" << sv << "'"
                           << endf;
                }
              case S_STRING:
                {
                  val = move (sv);
                  break;
                }
              default:
                break;
              }
              break;
            }
          case S_UNKNOWN:
            assert (false);
          }
        }
      }

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

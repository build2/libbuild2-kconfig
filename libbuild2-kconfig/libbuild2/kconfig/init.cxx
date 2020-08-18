#include <libbuild2/kconfig/init.hxx>

#include <libbuild2/file.hxx>
#include <libbuild2/scope.hxx>
#include <libbuild2/filesystem.hxx>
#include <libbuild2/diagnostics.hxx>

#include <libbuild2/config/utility.hxx>

#include <libbuild2/kconfig/lkc.h>

using namespace std;

namespace build2
{
  namespace kconfig
  {
    static const path def_file ("Kconfig");
    static const path val_file ("config.kconfig");

    static path
    configure (scope& rs, const location& l, const path& df)
    {
      context& ctx (rs.ctx);

      auto& vp (rs.var_pool ());

      // The config.kconfig controls the configuration method. Recognized
      // values are:
      //
      // ask
      //     conf --oldaskconfig
      //
      //     Update existing configuration (or create new if none exists)
      //     using existing values as default answers.
      //
      // old
      //     conf --oldconfig
      //
      //     Update existing configuration asking only for newly defined
      //     options.
      //
      // def
      //     conf --olddefconfig
      //
      //     Update existing configuration setting newly defined options to
      //     their default values.
      //
      // new-def [<file>]
      //     conf --alldefconfig | --defconfig <file>
      //
      //     Create a new configuration setting all options to values from
      //     <file> if specified and to their default values otherwise.
      //
      // qconf
      //     qconf
      //
      //     Update existing configuration (or create new if none exists)
      //     with graphical interface using existing values as defaults.
      //
      // mconf
      //     mconf
      //
      //     Update existing configuration (or create new if none exists)
      //     with menu interface using existing values as defaults.
      //
      // env <prog> [<args>]
      //     <prog> <args> Kconfig
      //
      //     Run <prog> in the configuration environment (KCONFIG_CONFIG, etc)
      //     passing the Kconfig definition file as the last argument. For
      //     example:
      //
      //     config.kconfig="env kconfig-conf --savedefconfig defconfig.kconfig"
      //
      const variable& c_k (vp.insert<strings> ("config.kconfig"));

      // Note: always transient and only entering during configure.
      //
      config::unsave_variable (rs, c_k);

      // We have a complication: we need to load the configuration here and
      // now since anything after (remainder of root.build, buildfiles) can
      // depend on its values. On the other hand, the overall configure meta-
      // operation may fail (for various reasons, including because the
      // configuration is incorrect as determined by what we load after) and
      // in this case we don't want to have config.kconfig "saved", similar to
      // config.build. Yet, on the third hand, we don't want to just discard
      // the configuration since it may have required a lot of effort to
      // create (and the failure may have nothing to do with it). So here we
      // are going to save the configuration as config.kconfig.new and in the
      // post-configure hook we will move it to config.kconfig. We will also
      // check if there is config.kconfig.new laying around and use that as
      // the starter configuration.
      //
      // Also a few notes on the configurator semantics: conf's --oldconfig
      // and --oldaskconfig are quite similar in that they both load the
      // existing configuration if present. The difference is that --oldconfig
      // only asks for "new" values (for some definition of "new") while
      // --oldaskconfig asks for all values using the existing configuration's
      // values as the default answers.
      //
      // The other configurators (mconf, qconf) implement a hybrid of the two
      // modes in that they also load the existing configuration if any and
      // then present the configuration tree/menu with this configuration's
      // values letting the user tweak what they deem necessary.
      //
      // @@ TODO: support customzing mode/method via config.kconfig?
      //
      // @@ COMP: will need to aggregate/disaggregate both files?

      path vf (rs.out_path () / rs.root_extra->build_dir / val_file + ".new");

      // If config.kconfig.new already exists, use that. Otherwise, if
      // config.kconfig exists, rename it to config.kconfig.new.
      //
      // Note that in the former case we may also have the .new.old file which
      // we should keep since configurators only create it if there are
      // changes.
      //
      bool e (false);
      if (!exists (vf))
      {
        path f (vf.base ());
        if ((e = exists (f)))
        {
          mvfile (f, vf, 2);
          rmfile (ctx, vf + ".old", 3); // For good measure.
        }
      }

      // Prepare the environment.
      //
      strings vars;
      vars.push_back ("KCONFIG_CONFIG=" + vf.string ()); // Keep it first.

      // Handle various configuration methods.
      //
      string conf;
      cstrings args {nullptr};

      if (const strings* pms = cast_null<strings> (rs[c_k]))
      {
        const strings& ms (*pms);

        if (ms.empty ())
          fail << "configuration method expected in " << c_k;

        const string& m (ms[0]);

        size_t n (1);
        if (m == "qconf" ||
            m == "mconf")
        {
          conf = m;
          args.push_back ("-s");
        }
        else if (m == "env")
        {
          conf = m;
          n = ms.size ();

          if (n == 1)
            fail << "expected program for configuration method env in " << c_k;

          args[0] = ms[1].c_str ();

          for (size_t i (2); i != n; ++i)
            args.push_back (ms[i].c_str ());
        }
        else
        {
          conf = "conf";
          args.push_back ("-s");

          if (m == "ask")
          {
            args.push_back ("--oldaskconfig");
          }
          else if (m == "old")
          {
            args.push_back ("--oldconfig");
          }
          else if (m == "def")
          {
            args.push_back ("--olddefconfig");
          }
          else if (m == "new-def")
          {
            if (ms.size () != 1)
            {
              args.push_back ("--defconfig");
              args.push_back (ms[1].c_str ());
              n = 2;
            }
            else
              args.push_back ("--alldefconfig");
          }
          else
            fail << "unknown configuration method '" << m << "' in " << c_k;
        }

        if (ms.size () > n)
          fail << "unexpected argument '" << ms[n] << "' for method " << m
               << " in " << c_k;
      }
      else
      {
        // What should the default configuration mode be? If the configuration
        // doesn't exist, then --oldaskconfig. If the configuration does
        // exists, then update it with --oldconfig. What if we have .new?
        // Initially we treated it as the existing case but if some buildfile
        // flags it as invalid, then on reconfigure we don't get a chance to
        // change anything (since from --oldconfig's point of view nothing has
        // changed). So now we use --oldaskconfig for this case.
        //
        conf = "conf";
        args.push_back ("-s");
        args.push_back (e ? "--oldconfig" : "--oldaskconfig");
      }

      // Resolve the configurator program.
      //
      process_env env;

      if (conf == "env")
        env = process_env (run_search (args[0]), vars);
      else
      {
        //@@ Before importing and running the configurator (for some modes),
        //   we could check ourselves whether existing configuration is good.
        //
        //   We could also fallback to built-in implementation (for some
        //   basic modes) if unable to import.
        //
        pair<const exe*, import_kind> ir (
          import_direct<exe> (
            rs,
            name ("kconfig-" + conf, dir_path (), "exe", "kconfig-" + conf),
            true      /* phase2 */,
            false     /* optional */,
            false     /* metadata */,
            l,
            "module load"));

        //@@ If we were to support normal import, we would have to update the
        //   target. Maybe one day.
        //
        if (ir.second != import_kind::adhoc)
          fail (l) << "project import of kconfig-" << conf << " not supported" <<
            info << "use config.kconfig_" << conf << " to specify executable path";

        const process_path& pp (ir.first->process_path ());
        args[0] = pp.recall_string ();

        env = process_env (pp, vars);
      }

      args.push_back (df.string ().c_str ());
      args.push_back (nullptr);

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

      return vf;
    }

    static bool
    configure_post (action a, const scope& rs)
    {
      if (a.operation () == default_id)
      {
        // Move config.kconfig.new to config.kconfig (see configure() above
        // for details).
        //
        path vf (rs.out_path () / rs.root_extra->build_dir / val_file);
        path nf (vf + ".new");

        mvfile (nf, vf, 1);

        nf += ".old";
        if (exists (nf))
        {
          vf += ".old";
          mvfile (nf, vf, 1);
        }

        return true;
      }

      return false;
    }

    static bool
    disfigure_pre (action a, const scope& rs)
    {
      if (a.operation () == default_id)
      {
        // Move config.kconfig to config.kconfig.old (see configure() above
        // for details).
        //
        path vf (rs.out_path () / rs.root_extra->build_dir / val_file);

        // Similar logic to configure(): if we have .new.old, use that
        // (unfinished configure), otherwise, use .kconfig.
        //
        path f (vf + ".new");
        bool e (exists (f));
        if (!e)
        {
          f.make_base ();
          e = exists (f);
        }

        if (e)
        {
          mvfile (f, vf + ".old", 1);
          return true;
        }
      }

      return false;
    }

    void
    boot (scope& rs, const location& l, module_boot_extra& extra)
    {
      tracer trace ("kconfig::init");
      l5 ([&]{trace << "for " << rs;});

      context& ctx (rs.ctx);

      // Register configure/disfigure hooks.
      //
      {
        bool c (      ctx.bootstrap_meta_operation ("configure"));
        bool d (!c && ctx.bootstrap_meta_operation ("disfigure"));
        if (c || d)
        {
          if (!(c
                ? config::configure_post (rs, &configure_post)
                : config::disfigure_pre  (rs, &disfigure_pre)))
            fail (l) << "config module must be loaded before kconfig";
        }
      }

      // Initialize after (or explicitly during) loading root.build.
      //
      extra.init = module_boot_init::after;
    }

    bool
    init (scope& rs,
          scope&,
          const location& l,
          bool first,
          bool,
          module_init_extra&)
    {
      tracer trace ("kconfig::init");
      l5 ([&]{trace << "for " << rs;});

      if (!first)
      {
        warn (l) << "multiple kconfig module initializations";
        return true;
      }

      context& ctx (rs.ctx);

      path df (rs.src_path () / rs.root_extra->build_dir / def_file);

      if (!exists (df))
        fail (l) << df << " does not exist";

      // (Re)configure if we are configuring.
      //
      path vf (ctx.current_mif->id == configure_id
               ? configure (rs, l, df)
               : rs.out_path () / rs.root_extra->build_dir / val_file);

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
      {"kconfig", &boot,  &init},
      {nullptr,   nullptr, nullptr}
    };

    const module_functions*
    build2_kconfig_load ()
    {
      return mod_functions;
    }
  }
}

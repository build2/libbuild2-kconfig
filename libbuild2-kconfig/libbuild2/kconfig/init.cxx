#include <libbuild2/kconfig/init.hxx>

#include <cstring> // strcmp() strlen()

#include <libbuild2/file.hxx>
#include <libbuild2/scope.hxx>
#include <libbuild2/function.hxx>
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

    // Currently, the Kconfig machinery only recognizes paths with forward
    // slashes. While this can probably be fixed, forming a path (say, in the
    // source directive) in a platform-aware way is unlikely to ever be
    // possible. So, at least for now, let's keep things simple and consistent
    // by passing paths with forward slahes on Windows.
    //
    // Note that we leave it up to the user to convert any other path
    // variables being passed, if necessary.
    //
    // Note also that for dir_path we return string, not representation (so no
    // trailing slash). It probably won't be helpful to include the slash
    // given the macro expansion semantics of Kconfig.
    //
    //
#ifndef _WIN32
    template <typename P>
    static inline const string&
    env_path (const P& p)
    {
      return p.string ();
    }
#else
    template <typename P>
    static inline string
    env_path (const P& p)
    {
      P t (p);
      t.canonicalize ('/');
      return move (t).string ();
    }
#endif

    // Convert Kconfig.* variable value to Kconfig environment value.
    //
    // This is fairly performance-sensitive so we try to optimize things a
    // bit.
    //
    static const string&
    env_convert (const scope& rs,
                 const variable& var, const value& val,
                 string& storage)
    {
      try
      {
        if (val.type == nullptr)
        {
          // Optimize the common case.
          //
          const names& ns (val.as<names> ());

          if (ns.size () == 1 && ns[0].simple ())
            return ns[0].value;

          storage = convert<string> (val);
        }
        else if (val.type->is_a<string> ())
        {
          return val.as<string> ();
        }
        else if (val.type->is_a<path> ())
        {
          return val.as<path> ().string ();
        }
        else if (val.type->is_a<dir_path> ())
        {
          // Note: not representation (see env_path() for details).
          //
          return val.as<dir_path> ().string ();
        }
        else if (val.type->is_a<project_name> ())
        {
          return val.as<project_name> ().string ();
        }
        else
        {
          // For other typed values call string() for conversion.
          //
          value tmp (val);
          storage = convert<string> (
            rs.ctx.functions.call (&rs,
                                   "string",
                                   vector_view<value> (&tmp, 1),
                                   location ()));
        }
      }
      catch (const invalid_argument& e)
      {
        fail << "unable to convert " << var << " value to string: " << e;
      }

      return storage;
    }

    struct env_data
    {
      const scope& rs;
      const location& loc;
      small_vector<string, 16> evars;
#ifdef _WIN32
      optional<string> src_root;
#endif
    };

    extern "C" char*
    build2_kconfig_getenv (const char* name, void* vd)
    {
      env_data& d (*static_cast<env_data*> (vd));
      const scope& rs (d.rs);

      optional<const char*> r;

      if (strcmp (name, "SRC_ROOT") == 0)
      {
#ifndef _WIN32
        r = env_path (rs.src_path ()).c_str ();
#else
        if (!d.src_root)
          d.src_root = env_path (rs.src_path ());

        r = d.src_root->c_str ();
#endif
      }
      else
      {
        auto& vs (d.evars);
        size_t n (strlen (name));

        // First check the cache.
        //
        auto i (find_if (vs.begin (), vs.end (),
                         [name, n] (const string& s)
                         {
                           return s.compare (0, n, name) == 0 && s[n] == '=';
                         }));

        if (i != vs.end ())
        {
          r = i->c_str () + n + 1;
        }
        else
        {
          // Lookup and, if it's expensive to convert, cache.
          //
          auto& vp (rs.ctx.var_pool);

          if (const variable* var = vp.find (string ("Kconfig.") + name))
          {
            lookup l (rs.vars[var]);

            if (l->null)
              r = nullptr;
            else
            {
              string s;
              const string& v (env_convert (rs, *var, *l, s));

              if (&v != &s)
                r = v.c_str ();
              else
              {
                s.insert (0, 1, '=');
                s.insert (0, name, n);
                vs.push_back (move (s));
                r = vs.back ().c_str () + n + 1;
              }
            }
          }
        }
      }

      if (!r)
      {
        // It's actually not clear this is a good idea since the variables
        // appear to be expanded ignoring conditions, for example:
        //
        // default "$(FOO)" if BAR
        //
        // FOO will be looked up even if BAR is false. So let's see how it
        // goes, we may have to change this.
        //
        fail << "undefined Kconfig variable " << name <<
          info (d.loc) << "consider setting Kconfig." << name << " variable "
             << "before loading kconfig module";
      }

      return const_cast<char*> (*r);
    }

    // Collect Kconfig environment variables.
    //
    // Note: KCONFIG_CONFIG is expected to be first.
    //
    static strings
    env_init (const scope& rs, const path& vf, const variable& var_K)
    {
      context& ctx (rs.ctx);
      auto& vp (ctx.var_pool);

      // @@ TODO: maybe this should be customizable (kconfig.kconfig.title?)
      //
      string title;
      const project_name& prj (project (rs));
      if (!prj.empty ())
      {
        title = prj.string ();

        if (const string* ver = cast_null<string> (rs[ctx.var_version]))
        {
          title += ' ';
          title += *ver;
        }
      }

      strings evars;

      evars.push_back ("KCONFIG_CONFIG=" + env_path (vf));
      evars.push_back ("SRC_ROOT=" + env_path (rs.src_path ()));

      if (!title.empty ())
        evars.push_back ("KCONFIG_MAINMENU=" + title);

      // Add Kconfig.* variables if any.
      //
      for (auto p (rs.vars.lookup_namespace (var_K));
           p.first != p.second;
           ++p.first)
      {
        const variable* var (&p.first->first.get ());

        // Annoyingly, this can be one of the overrides (__override,
        // __prefix, etc).
        //
        if (size_t n = var->override ())
          var = vp.find (string (var->name, 0, n));

        const value& val (p.first->second);

        if (val.null)
          continue;

        string s;
        const string& v (env_convert (rs, *var, val, s));

        s.insert (0, 1, '=');
        s.insert (0, var->name, 8, string::npos);

        if (&v != &s)
          s += v;

        evars.push_back (move (s));
      }

      return evars;
    }

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
      // The kconfig variable can be set by the project to select the default
      // method for creating new configurations. For example:
      //
      // kconfig = new-def $src_root/build/deconfig.kconfig
      //
      const variable& var_c_k (vp.insert<strings> ("config.kconfig"));
      const variable& var_k   (vp.insert<strings> ("kconfig", false /*ovr*/));

      // Kconfig.* variable prefix.
      //
      const variable& var_K (vp.insert ("Kconfig"));

      // Note: always transient and only entering during configure.
      //
      config::unsave_variable (rs, var_c_k);

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
      // @@ COMP: will need to aggregate/disaggregate both files?

      path vf (rs.out_path () / rs.root_extra->build_dir / val_file + ".new");

      // If config.kconfig.new already exists, use that. Otherwise, if
      // config.kconfig exists, rename it to config.kconfig.new.
      //
      // Note that in the former case we may also have the .new.old file which
      // we should keep since configurators only create it if there are
      // changes.
      //
      enum {exist_none, exist_cur, exist_new} e (exist_none);
      if (exists (vf))
        e = exist_new;
      else
      {
        path f (vf.base ());
        if (exists (f))
        {
          mvfile (f, vf, 2);
          rmfile (ctx, vf + ".old", 3); // For good measure.
          e = exist_cur;
        }
      }

      // Prepare the environment.
      //
      strings evars (env_init (rs, vf, var_K));

      // Handle various configuration methods.
      //
      string conf;
      cstrings args {nullptr};
      string arg_df;
      string arg_cf;

      auto process_method = [&conf, &args, &arg_cf] (const strings& ms,
                                                     const variable& var)
      {
        if (ms.empty ())
          fail << "configuration method expected in " << var;

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
            fail << "expected program for configuration method env in " << var;

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
              args.push_back ((arg_cf = env_path (path (ms[1]))).c_str ());
              n = 2;
            }
            else
              args.push_back ("--alldefconfig");
          }
          else
            fail << "unknown configuration method '" << m << "' in " << var;
        }

        if (ms.size () > n)
          fail << "unexpected argument '" << ms[n] << "' for method " << m
               << " in " << var;
      };

      {
        const strings* m (cast_null<strings> (rs[var_c_k]));

        if (m != nullptr)
        {
          process_method (*m, var_c_k);
        }
        else if (e == exist_none &&
                 (m = cast_null<strings> (rs[var_k])) != nullptr)
        {
          process_method (*m, var_k);
        }
        else
        {
          // What should the default configuration mode be? If the it doesn't
          // exist, then --oldaskconfig. If it does exists, then update it
          // with --oldconfig. What if we have .new?  Initially we treated it
          // as the existing case but if some buildfile flags it as invalid,
          // then on reconfigure we don't get a chance to change anything
          // (since from --oldconfig's point of view nothing has changed). So
          // now we use --oldaskconfig for this case.
          //
          conf = "conf";
          args.push_back ("-s");
          args.push_back (e == exist_cur ? "--oldconfig" : "--oldaskconfig");
        }
      }

      // Resolve the configurator program.
      //
      process_env env;

      if (conf == "env")
        env = process_env (run_search (args[0]), evars);
      else
      {
        //@@ Before importing and running the configurator (for some modes),
        //   we could check ourselves whether existing configuration is good.
        //
        //@@ We could also fallback to built-in implementation (for some
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

        env = process_env (pp, evars);
      }

      args.push_back ((arg_df = env_path (df)).c_str ());
      args.push_back (nullptr);

      if (verb >= 3)
        print_process (env, args);
      else if (verb >= 2)
      {
        diag_record dr (text);
        dr << evars[0] << ' '; print_process (dr, args);
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
      tracer trace ("kconfig::boot");
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

      auto conf_g (make_guard ([] () {conf_free ();}));

      // Disable extra diagnostics.
      //
      conf_set_message_callback (nullptr);

      // Resolve getenv() calls as Kconfig.* lookups.
      //
      env_data edata {rs, l, {}};
      conf_set_getenv_callback (&build2_kconfig_getenv, &edata);

      // Load the configuration definition (Kconfig).
      //
      conf_parse (env_path (df).c_str ());

      // Load the configuration values.
      //
      if (!exists (vf))
        fail (l) << vf << " does not exist" <<
          info << "consider configuring " << rs.out_path ();

      if (conf_read (env_path (vf).c_str ()) != 0)
        fail (l) << "unable to load " << vf;

      if (conf_get_changed ())
        fail (l) << "Kconfig configuration definition/values have changed" <<
          info << "reconfigure to synchronize" <<
          info << "configuration definition: " << df <<
          info << "configuration values: " << vf;

      // Set Kconfig symbols as kconfig.* variables.
      //
      {
        auto& vp (rs.var_pool ());

        size_t i;
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
          bool var_q (false);
          string var_n (string ("kconfig.") + s->name);
          lcase (var_n);

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

                  fail (l) << "invalid Kconfig int value '" << sv << "'"
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

                  fail (l) << "invalid Kconfig hex value '" << sv << "'"
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
      // Unset Kconfig environment variables that may interfere with our
      // business. These we allow through:
      //
      // KCONFIG_PROBABILITY
      // ZCONF_DEBUG
      // KCONFIG_SEED          (conf --randconfig)
      // KCONFIG_ALLCONFIG     (conf --randconfig)
      // NCONFIG_MODE          (nconf)
      // MENUCONFIG_MODE       (mconf)
      // MENUCONFIG_COLOR      (mconf)
      //
      // These don't seem to affect us:
      //
      // KCONFIG_NOSILENTUPDATE  (conf --syncconfig)
      // KCONFIG_CONFIG          (always reset)
      // KCONFIG_MAINMENU        (always reset)
      try
      {
        unsetenv ("srctree");
        unsetenv ("CONFIG_");
        unsetenv ("KCONFIG_AUTOCONFIG");
        // unsetenv ("KCONFIG_AUTOHEADER"); // Disabled by KCONFIG_AUTOCONFIG.
        unsetenv ("KCONFIG_OVERWRITECONFIG");
      }
      catch (const system_error& e)
      {
        error << "unable to clear Kconfig environment: " << e;
        return nullptr;
      }

      return mod_functions;
    }
  }
}

#include <libbuild2/kconfig/init.hxx>

#include <cstring> // strcmp() strlen()
#include <iomanip> // left, setw()

#include <libbuild2/file.hxx>
#include <libbuild2/scope.hxx>
#include <libbuild2/function.hxx>
#include <libbuild2/filesystem.hxx>
#include <libbuild2/diagnostics.hxx>

#include <libbuild2/dist/module.hxx>
#include <libbuild2/config/utility.hxx>

#include <libbuild2/kconfig/lkc.h>
#include <libbuild2/kconfig/confapi.h>

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
    // given the primitive macro expansion semantics of Kconfig.
    //
    //
#ifndef _WIN32
    template <typename K>
    static inline string
    env_path (basic_path<char, K>&& p)
    {
      return move (p).string ();
    }

    template <typename K>
    static inline const string&
    env_path (const basic_path<char, K>& p)
    {
      return p.string ();
    }
#else
    template <typename K>
    static inline string
    env_path (basic_path<char, K> p)
    {
      p.canonicalize ('/');
      return move (p).string ();
    }
#endif

    static inline string
    env_path (string s)
    {
      try
      {
        return env_path (path (move (s)));
      }
      catch (const invalid_path& e)
      {
        fail << "invalid path '" << e.path << "'" << endf;
      }
    }

    // Convert Kconfig.* variable value to Kconfig environment value.
    //
    // This is fairly performance-sensitive so we try to optimize things a
    // bit.
    //
    static const string ktrue ("y");
    static const string kfalse ("n");

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
        else if (val.type->is_a<bool> ())
        {
          return val.as<bool> () ? ktrue : kfalse;
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

    static string
    env_title (const scope& rs)
    {
      string r;

      const project_name& prj (project (rs));
      if (!prj.empty ())
      {
        r = prj.string ();

        if (const string* ver = cast_null<string> (rs[rs.ctx.var_version]))
        {
          r += ' ';
          r += *ver;
        }
      }

      return r;
    }

    struct env_data
    {
      const scope& rs;
      const location& loc;
      small_vector<string, 16> evars;
      optional<string> title;
#ifdef _WIN32
      optional<string> src_root;
#endif

      // We cannot throw from an extern "C" function so we return fail status.
      //
      bool failed = false;

      env_data (const scope& s, const location& l, const string* t)
          : rs (s), loc (l)
      {
        if (t != nullptr)
          title = *t;
      }
    };

    extern "C" char*
    build2_kconfig_getenv (const char* name, void* vd)
    {
      env_data& d (*static_cast<env_data*> (vd));
      const scope& rs (d.rs);

      optional<const char*> r;

      if (strcmp (name, SRCTREE) == 0                 ||
          strcmp (name, "CONFIG_") == 0               ||
          strcmp (name, "ZCONF_DEBUG") == 0           ||
          strcmp (name, "KCONFIG_OVERWRITECONFIG") == 0)
      {
        r = nullptr;
      }
      else if (strcmp (name, "SRC_ROOT") == 0)
      {
#ifndef _WIN32
        r = env_path (rs.src_path ()).c_str ();
#else
        if (!d.src_root)
          d.src_root = env_path (rs.src_path ());

        r = d.src_root->c_str ();
#endif
      }
      else if (strcmp (name, "KCONFIG_MAINMENU") == 0)
      {
        if (!d.title)
          d.title = env_title (rs);

        r = d.title->c_str ();
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
          // Note: go straight for the public variable pool.
          //
          auto& vp (rs.ctx.var_pool);

          if (const variable* var = vp.find (string ("Kconfig.") + name))
          {
            lookup l (rs.vars[var]);

            if (l->null)
              r = nullptr;
            else
            {
              try
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
              catch (const failed&)
              {
                r = nullptr;
                d.failed = true;
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
        error << "undefined Kconfig variable " << name <<
          info (d.loc) << "consider setting Kconfig." << name << " variable "
              << "before loading kconfig module";

        r = nullptr;
        d.failed = true;
      }

      return const_cast<char*> (*r);
    }

    // Collect Kconfig environment variables.
    //
    // Note: KCONFIG_CONFIG is expected to be first.
    //
    static strings
    env_init (const scope& rs, const path& vf, const variable& var_k_k_t)
    {
      context& ctx (rs.ctx);

      // Note: go straight for the public variable pool.
      //
      auto& vp (ctx.var_pool);

      strings evars;

      evars.push_back ("KCONFIG_CONFIG=" + env_path (vf));
      evars.push_back ("SRC_ROOT=" + env_path (rs.src_path ()));

      {
        string t;

        if (const string* v = cast_null<string> (rs[var_k_k_t]))
          t = *v;
        else
          t = env_title (rs);

        if (!t.empty ())
          evars.push_back ("KCONFIG_MAINMENU=" + t);
      }

      // Add Kconfig.* variables if any.
      //
      for (auto p (rs.vars.lookup_namespace ("Kconfig"));
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
    configure (scope& rs, const location& l,
               const path& df,
               const variable& var_k_k_t)
    {
      context& ctx (rs.ctx);

      // All the variables we enter are qualified so go straight for the
      // public variable pool.
      //
      auto& vp (rs.var_pool (true /* public */));

      // The config.kconfig variable controls the configuration method. Its
      // value has the following structure:
      //
      // [(new|old)-](def|ask|reask|mconf|qconf|env) [...]
      //
      // The optional new|old component controls the reuse of existing
      // config.kconfig. If new is specified, then the configuration is
      // created from scratch (with existing config.kconfig, if any, saved as
      // config.kconfig.old). If old is specified, then the configuration is
      // created based on existing config.kconfig (with what "based" means
      // determined by the second component). In this case, it's an error for
      // there to be no existing config.kconfig. If the new|old component is
      // omitted, then existing config.kconfig is reused if present (old
      // semantics) and created from scratch otherwise (new semantics).
      //
      // Given a configuration option definition in the Kconfig file, it's
      // value can come from the following sources:
      //
      // 1. Existing config.kconfig.
      //
      // 2. Default configuration file.
      //
      // 3. Default value from Kconfig.
      //
      // A configuration option that does not have a value in existing
      // config.kconfig is referred to as a newly-defined configuration
      // option. Naturally, if config.kconfig does not exist or is not used
      // (because of the new first component), then all options are
      // newly-defined.
      //
      // The second component determines the configurator and its mode and has
      // the following possible values:
      //
      // def [<file>]
      //
      //     Set newly-defined configuration options to values from <file> if
      //     specified and to their default values from Kconfig otherwise (if
      //     some options are missing in <file>, they are set to default
      //     values as well). To reset all the configuration options, use
      //     new-def.
      //
      // ask [<file>]
      //
      //     Ask for values of only newly-defined configuration options. For
      //     default answers use values from <file> if specified and default
      //     values from Kconfig otherwise.
      //
      //     Note that if config.kconfig does not exist or is not used
      //     (new-ask) then this method is equivalent to reask.
      //
      // reask [<file>]
      //
      //     Ask for values of all the configuration options. For default
      //     answers use values from config.kconfig if exists, from <file> if
      //     specified, and default values from Kconfig otherwise.
      //
      // mconf
      //
      //     Present all the configuration options in a menu interface. For
      //     default answers, use values from config.kconfig if exists and
      //     default values from Kconfig otherwise.
      //
      // qconf
      //
      //     Present all the configuration options in a graphical interface.
      //     For default answers, use values from config.kconfig if exists and
      //     default values from Kconfig otherwise.
      //
      // env <prog> [<args>]
      //
      //     Run <prog> in the configuration environment (KCONFIG_CONFIG, etc)
      //     passing the Kconfig definition file as the last argument. For
      //     example:
      //
      //     config.kconfig="env kconfig-conf --savedefconfig defconfig.kconfig"
      //
      // A few notes on the configurator semantics: kconfig-conf --oldconfig
      // and --oldaskconfig are quite similar in that they both load the
      // existing configuration if present. The difference is that --oldconfig
      // only asks for newly-defined options while --oldaskconfig asks for all
      // options using the existing configuration's values as the default
      // answers.
      //
      // The other configurators (mconf, qconf) implement a hybrid of the two
      // modes in that they also load the existing configuration if any and
      // then present the configuration tree/menu with this configuration's
      // values letting the user tweak what they deem necessary.
      //
      // Mapping for some of the kconfig-conf options to the above methods:
      //
      // --oldconfig             ask
      // --oldaskconfig          reask
      // --olddefconfig          def
      // --alldefconfig      new-def
      // --defconfig <file>  new-def <file>
      //
      // Note that the following potentially useful methods have no
      // kconfig-conf equivalents:
      //
      // old-def <file>
      // [re]ask <file>
      //
      // The kconfig.kconfig.{configure,reconfigure,retryconfigure} variables
      // can be set by the project to select the default method for creating
      // new, updating existing, and trying to fix previously created/updated
      // configurations, respectively. For example:
      //
      // kconfig.kconfig.configure = new-def $src_root/build/defconfig.kconfig
      // kconfig.kconfig.reconfigure = old-def $src_root/build/defconfig.kconfig
      //
      // The default methods for these variables are reask, ask, and reask,
      // respectively (see below for rationale).
      //
      // The kconfig.kconfig.transient variable (entered in init()) can be set
      // by the project to select the transient configuration method. Setting
      // it to ask (which is also the default), disables the ability to use
      // transient configurations. For example:
      //
      // kconfig.kconfig.transient = def $src_root/build/deconfig.kconfig
      //
      // Note: there are two plausible defaults for theses four variables:
      // reask/ask/reask/ask and new-def/old-def/reask/def. We've settled on
      // the first one to make the decision to use the default values
      // explicit. It's also more consistent with kconfig-conf which does
      // reask by default (i.e., if none of the mode options is specified).
      //
      // @@ TODO: adhoc value for kconfig.kconfig.transient to allow ad hoc
      //    transient configuration (e.g., set manually in root.build). Set
      //    kconfig=[null] as an indication?
      //
      auto& var_c_k   (vp.insert<strings> ("config.kconfig"));
      auto& var_k_k_c (vp.insert<strings> ("kconfig.kconfig.configure"));
      auto& var_k_k_r (vp.insert<strings> ("kconfig.kconfig.reconfigure"));
      auto& var_k_k_y (vp.insert<strings> ("kconfig.kconfig.retryconfigure"));

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
      // @@ COMP: will need to aggregate/disaggregate both files?
      //
      path ef (rs.out_path () / rs.root_extra->build_dir / val_file);
      path vf (ef + ".new");
      path cf; // Default configuration file.

      // Determine what exists.
      //
      enum {exist_none, exist_cur, exist_new} e (
        exists (vf) ? exist_new :
        exists (ef) ? exist_cur : exist_none);

      // Handle various configuration methods.
      //
      // We translate the second component to the configurator program (conf,
      // mconf, qconf, and env) and its arguments. For methods that are not
      // supported by conf, we invent some imaginary mode arguments which
      // means that they can only be implemented as builtins.
      //
      string conf;
      cstrings args {nullptr};
      string arg_df;
      string arg_cf;
      {
        auto set_cf = [&cf, &arg_cf] (const string& f) -> const char*
        {
          try
          {
            cf = path (f);
          }
          catch (const invalid_path& e)
          {
            fail << "invalid path '" << e.path << "'" << endf;
          }

          arg_cf = env_path (cf);
          return arg_cf.c_str ();
        };

        const variable* v (&var_c_k);
        lookup l (rs[*v]);

        if (!l)
        {
          v = (e == exist_none ? &var_k_k_c :
               e == exist_cur  ? &var_k_k_r :
               e == exist_new  ? &var_k_k_y : nullptr);

          l = rs[*v];
        }

        if (l)
        {
          const strings& ms (cast<strings> (l));

          if (ms.empty ())
            fail << "configuration method expected in " << *v;

          string m (ms[0]);

          // Deal with the prefix.
          //
          bool old;
          if (m.compare (0, 4, "old-") == 0 ||
              m.compare (0, 4, "new-") == 0)
          {
            old = (m[0] == 'o');

            // Reconcile desired old/new with reality.
            //
            if (old)
            {
              if (e == exist_none)
                fail << "configuration method " << m << " requested but "
                     << ef << " does not exist";
            }
            else if (e != exist_none)
            {
              // Note that we always move it to config.kconfig.new.old to be
              // aligned with the two-stage process (see configure_post()).
              //
              mvfile (e == exist_new ? vf : ef, vf + ".old", 2);
              e = exist_none;
            }

            m.erase (0, 4);
          }
          else
            old = (e != exist_none);

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
              fail << "expected program for configuration method env in " << *v;

            args[0] = ms[1].c_str ();

            for (size_t i (2); i != n; ++i)
              args.push_back (ms[i].c_str ());
          }
          else
          {
            // Note: mode option position must be 2.
            //
            conf = "conf";
            args.push_back ("-s");

            if (m == "def")
            {
              if (old)
              {
                args.push_back ("--olddefconfig");

                if (ms.size () != 1)
                {
                  args.push_back (set_cf (ms[1]));
                  n = 2;
                }
              }
              else
              {
                if (ms.size () != 1)
                {
                  args.push_back ("--defconfig");
                  args.push_back (set_cf (ms[1]));
                  n = 2;
                }
                else
                  args.push_back ("--alldefconfig");
              }
            }
            else if (m == "ask" || m == "reask")
            {
              // If there is no existing configuration, remap it to reask so
              // that we get better diagnostics from conf (no misleading
              // "Restart config..."  message).
              //
              args.push_back (m[0] == 'r' || e == exist_none
                              ? "--oldaskconfig"
                              : "--oldconfig");

              if (ms.size () != 1)
              {
                args.push_back (set_cf (ms[1]));
                n = 2;
              }
            }
            else
              fail << "unknown configuration method '" << m << "' in " << *v;
          }

          if (ms.size () > n)
            fail << "unexpected argument '" << ms[n] << "' for method " << m
                 << " in " << *v;
        }
        else
        {
          // What should the default configuration mode be? If it doesn't
          // exist, then --oldaskconfig. If it does exists, then update it
          // with --oldconfig. What if we have .new?  Initially we treated it
          // as the existing case but if some buildfile flags it as invalid,
          // then on reconfigure we don't get a chance to change anything
          // (since from --oldconfig's point of view nothing has changed). So
          // now we use --oldaskconfig for this case.
          //
          // Note: mode option position must be 2.
          //
          conf = "conf";
          args.push_back ("-s");
          args.push_back (e == exist_cur ? "--oldconfig" : "--oldaskconfig");
        }
      }

      args.push_back ((arg_df = env_path (df)).c_str ());
      args.push_back (nullptr);

      // If config.kconfig.new already exists, we use that. Otherwise, if
      // config.kconfig exists, rename it to config.kconfig.new.
      //
      // Note that in the former case we may also have the .new.old file which
      // we should keep since configurators only create it if there are
      // changes.
      //
      if (e == exist_cur)
      {
        mvfile (ef, vf, 2);
        rmfile (ctx, vf + ".old", 3); // For good measure.
      }

      // Handle some conf modes as built-in. The main advantage is that we
      // don't need the presence of the kconfig-conf executable.
      //
      // Note that the performance is not too important here so we don't try
      // to optimize things too much. Specifically, we keep it as args so that
      // we can easily switch back to the executable (say, for debugging).
      //
      // Also, we could have "returned" the kconfig state saving the caller
      // the conf_parse/read() calls. But that would complicate the interface
      // plus the re-load serves as an extra sanity check (the configuration
      // we produce should not be out of date).
      //
      string mode;
      if (conf == "conf" && ((mode = args[2]) == "--defconfig"    ||
                             (mode          ) == "--olddefconfig" ||
                             (mode          ) == "--alldefconfig" ||
                             (mode          ) == "--oldaskconfig" ||
                             (mode          ) == "--oldconfig"))
      {
        args[0] = "kconfig-conf";
        string arg_vf (env_path (vf));

        if (verb >= 2)
        {
          diag_record dr (text);
          dr << "KCONFIG_CONFIG=" << arg_vf << ' '; print_process (dr, args);
        }
        else if (verb)
          print_diag ("kconfig", vf);

        // Similar code to init() below.
        //
        auto conf_g (make_guard ([] () {conf_free ();}));

        conf_set_message_callback (nullptr);

        env_data edata (rs, l, cast_null<string> (rs[var_k_k_t]));
        conf_set_getenv_callback (&build2_kconfig_getenv, &edata);

        conf_parse (arg_df.c_str ());
        if (edata.failed)
          throw failed ();

        // Load the existing configuration for --old* modes.
        //
        if (mode.compare (0, 5, "--old") == 0)
        {
          // See if we have the default configuration file.
          //
          if (args.size () != 6)
          {
            if (e != exist_none && conf_read (arg_vf.c_str ()) != 0)
              fail (l) << "unable to load " << vf;
          }
          else
          {
            // There is no equivalent functionality in kconfig-conf so we have
            // to assemble something together ourselves.
            //
            auto sym_move_val = [] (symbol* s, int src, int dst)
            {
              switch (s->type)
              {
              case S_INT:
              case S_HEX:
              case S_STRING:
                s->def[dst].val = s->def[src].val;
                s->def[src].val = nullptr;
                // Fall through.
              case S_BOOLEAN:
              case S_TRISTATE:
                s->def[dst].tri = s->def[src].tri;
                break;
              case S_UNKNOWN:
                assert (false);
              }

              s->flags &= ~(SYMBOL_DEF << src);
              s->flags |=  (SYMBOL_DEF << dst);
            };

            // First load the default configuration file.
            //
            // While it feels like S_DEF_DEF3/4 are there for exactly this
            // kind of thing, there are subtle differences in semantics of
            // conf_read_simple() that we'd rather not mess with. So instead
            // we are going to load it as S_DEF_USER (which is how, say,
            // --defconfig does it) and then manually move the values to
            // S_DEF_DEF3.
            //
            conf_set_changed (false);

            assert (args[3] == arg_cf.c_str ());
            if (conf_read_simple (args[3], S_DEF_USER) != 0)
              fail (l) << "unable to load " << cf;

            // Omit moving things back and forth if there is not current
            // configuration. Essentially, this becomes a conf_read() call.
            //
            if (e != exist_none)
            {
              size_t i;
              symbol* s;
              for_all_symbols (i, s)
              {
                if ((s->flags & SYMBOL_DEF_USER) == 0 || s->type == S_UNKNOWN)
                  continue;

                sym_move_val (s, S_DEF_USER, S_DEF_DEF3);
              }

              // Next load the current configuration file.
              //
              // Note that we don't want any changes in the default file to
              // count so we reset the flag.
              //
              conf_set_changed (false);

              if (conf_read_simple (arg_vf.c_str (), S_DEF_USER) != 0)
                fail (l) << "unable to load " << vf;

              // Now go over all the symbols and move S_DEF_DEF3 values to
              // S_DEF_USER for symbols that don't already have S_DEF_USER.
              //
              for_all_symbols (i, s)
              {
                if ((s->flags & SYMBOL_DEF3) == 0)
                  continue;

                // Choices are tricky (see conf_read_simple() for some
                // meditation material). Specifically, what is stored in the
                // config file is the selected choice arm (along with
                // commented out non-selected arms). So we should only
                // propagate the selection from the default file if the choice
                // hasn't been selected by any arm from the currect file.
                // conf_read_simple() doesn't mark the choice symbol as
                // "selected" so to be safe we will only propagate our value
                // if none of its other arms have been mentioned in the
                // current file with yes or mod.
                //
                // Note that from studying conf_write() it appears that choice
                // symbols themselves are never written. They are also marked
                // with SYMBOL_DEF* by conf_read_simple().
                //
                if (sym_is_choice (s))
                  continue;
                else if (sym_is_choice_value (s))
                {
                  auto sym_choice = [] (symbol* cv)
                  {
                    return prop_get_symbol (sym_get_choice_prop (cv));
                  };

                  symbol* cs (sym_choice (s));

                  // First see if there is any mentioning of this choice's
                  // arms in S_DEF_USER.
                  //
                  size_t i1;
                  symbol* s1;
                  for_all_symbols (i1, s1)
                  {
                    if ((s1->flags & SYMBOL_DEF_USER) != 0 &&
                        sym_is_choice_value (s1)           &&
                        sym_choice (s1) == cs              &&
                        s1->def[S_DEF_USER].tri != no)
                    {
                      cs = nullptr;
                      break;
                    }
                  }

                  if (cs == nullptr)
                    continue;

                  // Move values of this choice and the choice itself all at
                  // once (so we don't confuse partially moved with current
                  // choice symbols). Note that the choice itself may not be
                  // marked with SYMBOL_DEF3 (see conf_read_simple()).
                  //
                  for_all_symbols (i1, s1)
                  {
                    if ((s1->flags & SYMBOL_DEF3) != 0 &&
                        sym_is_choice_value (s1)       &&
                        sym_choice (s1) == cs)
                      sym_move_val (s1, S_DEF_DEF3, S_DEF_USER);
                  }

                  sym_move_val (cs, S_DEF_DEF3, S_DEF_USER);
                }
                else
                {
                  if ((s->flags & SYMBOL_DEF_USER) != 0)
                    continue;

                  sym_move_val (s, S_DEF_DEF3, S_DEF_USER);
                }
              }
            }

            // Finally execute the tail of conf_read() to process the values.
            //
            conf_read ("");
          }
        }

        if (mode == "--defconfig")
        {
          assert (args[3] == arg_cf.c_str ());
          if (conf_read (args[3]) != 0)
            fail (l) << "unable to load " << cf;

          conf_set_all_new_symbols (def_default);
        }
        else if (mode == "--olddefconfig")
        {
          // Note that this call is missing in conf.c for some reason even
          // though the documentation implies it should be there.
          //
          conf_set_all_new_symbols (def_default);
        }
        else if (mode == "--alldefconfig")
        {
          conf_set_all_new_symbols (def_default);
        }
        else if (mode == "--oldaskconfig")
        {
          conf_reask ();
        }
        else if (mode == "--oldconfig")
        {
          conf_ask ();
        }
        else
          assert (false);

        if (conf_write (arg_vf.c_str ()) != 0)
          fail (l) << "unable to save " << vf;

        // While we shouldn't throw for any variable that conf_write() might
        // query, let's check for good measure (e.g., if it queries something
        // new).
        //
        if (edata.failed)
          throw failed ();
      }
      else
      {
        // Prepare the environment.
        //
        strings evars (env_init (rs, vf, var_k_k_t));

        // Unset Kconfig environment variables that may interfere with our
        // business (we only do that if they are present to keep diagnostics
        // tidy).
        //
        // These we allow through (and some save in hermetic configurations):
        //
        // KCONFIG_PROBABILITY
        // ZCONF_DEBUG
        // KCONFIG_SEED          (conf --randconfig)
        // KCONFIG_ALLCONFIG     (conf --randconfig)
        // NCONFIG_MODE          (nconf)
        // MENUCONFIG_MODE       (mconf)
        // MENUCONFIG_COLOR      (mconf)
        //
        // These don't affect us:
        //
        // KCONFIG_NOSILENTUPDATE  (conf --syncconfig)
        // KCONFIG_CONFIG          (always reset)
        // KCONFIG_MAINMENU        (always reset)
        //
        // Note that for the built-in modes we cut off the outside environment
        // entirely via the getenv() callback (and those few calls in conf.c
        // that don't got through the callback do not happen in the build-in
        // modes).
        //
        auto unsetenv = [&evars] (const char* n)
        {
          if (getenv (n))
            evars.push_back (n);
        };

        unsetenv (SRCTREE);
        unsetenv ("CONFIG_");
        unsetenv ("KCONFIG_AUTOCONFIG");
        // unsetenv ("KCONFIG_AUTOHEADER"); // Disabled by KCONFIG_AUTOCONFIG.
        unsetenv ("KCONFIG_OVERWRITECONFIG");
        unsetenv ("KCONFIG_DEFCONFIG_LIST");

        // Resolve the configurator program.
        //
        process_path pp;
        process_env env;

        if (conf == "env")
        {
          pp = run_search (args[0]);
          env = process_env (pp, evars);
        }
        else
        {
          import_result<exe> ir (
            import_direct<exe> (
              rs,
              name ("kconfig-" + conf, dir_path (), "exe", "kconfig-" + conf),
              true      /* phase2 */,
              false     /* optional */,
              false     /* metadata */,
              l,
              "module load"));

          // If we were to support normal import, we would have to update the
          // target. Maybe one day.
          //
          if (ir.kind != import_kind::adhoc)
            fail (l) << "project import of kconfig-" << conf << " not supported" <<
              info << "use config.kconfig_" << conf << " to specify executable path";

          const process_path& pp (ir.target->process_path ());
          args[0] = pp.recall_string ();

          env = process_env (pp, evars);
        }

        if (verb >= 3)
          print_process (env, args);
        else if (verb >= 2)
        {
          diag_record dr (text);
          dr << evars[0] << ' '; print_process (dr, args);
        }
        else if (verb)
          print_diag ("kconfig", vf);

        run (ctx, env, args, 1 /* finish_verbosity */);
      }

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

    // Return true if this is a "real" symbol that we need to convert to the
    // kconfig.* value, report, etc.
    //
    static inline bool
    real_symbol (const symbol* s)
    {
      // Skip symbols corresponding to symbol-less choices (NULL names).
      //
      // Skip unknown types. There seems to be two kind of symbols that
      // appear to have the unknown type:
      //
      // 1. Symbols that are referenced (e.g., in `depends on`) but not
      //    defined.
      //
      // 2. Some sort of non-const values (typically integers).
      //
      // Skip constants (their names are the same as the value).
      //
      if (s->name == nullptr   ||
          s->type == S_UNKNOWN ||
          (s->flags & SYMBOL_CONST) != 0)
        return false;

      // Notes on symbols that don't have the SYMBOL_WRITE flags:
      //
      // 1. Almost all of them have false/NULL value. The one exception we
      //    have seen (DEFCONFIG_LIST) had the SYMBOL_NO_WRITE flag.
      //
      assert (s->flags & SYMBOL_VALID);

      return true;
    }

    // Recursively print the report in menu order/hierarchy. Return the number
    // of real symbols seen (used for the consistency check).
    //
    static size_t
    report (menu* m, diag_record& dr, size_t pad, size_t lev = 0)
    {
      size_t n (0);
      bool nest (false);

      // This is based on the logic found in kconfig-conf's conf() function.
      //
      if (const property* p = m->prompt)
      {
        switch (p->type)
        {
        case P_MENU:
          {
            // Skip the root menu (which is normally the project name and
            // version).
            //
            // Note also that this can be menuconfig in which case we will
            // have both the menu line and the symbol line.
            //
            if (m != &rootmenu && p->text != nullptr)
            {
              dr << "\n  " << string (lev * 2, ' ') << '>' << p->text;
              nest = true;
            }
            break;
          }
        case P_COMMENT:
          {
            if (p->text != nullptr && menu_is_visible (m))
            {
              dr << "\n  " << string (lev * 2, ' ') << '|' << p->text;
            }
            break;
          }
        default:
          break;
        }
      }

      if (symbol* s = m->sym)
      {
        if (!real_symbol (s))
          goto children;

        n++;

        // Let's omit printing NULL values (see init() below for details).
        //
        if (s->type != S_BOOLEAN && s->type != S_TRISTATE &&
            !(s->flags & SYMBOL_WRITE) && !(s->flags & SYMBOL_NO_WRITE))
          goto children;

        // Nesting choices doesn't seem worth it: most of them will have no
        // symbol and one true arm. We also don't nest implicit menuconfig
        // symbols. So nesting is determined by the presence of menu prompt.

        const symbol_value& v (s->curr);

        // Skip printing false choice arms.
        //
        if (sym_is_choice_value (s) && v.tri == no)
          goto children;

        dr << "\n  " << string (lev * 2, ' ')
           << left << setw (static_cast<int> (pad)) << lcase (s->name) << ' ';

        switch (s->type)
        {
        case S_BOOLEAN:
        case S_TRISTATE:
          {
            dr << (v.tri == yes ? "true" :
                   v.tri == mod ? "module" : "false");
            break;
          }
        case S_INT:
        case S_HEX:
        case S_STRING:
          {
            dr << static_cast<const char*> (v.val);
            break;
          }
        case S_UNKNOWN:
          assert (false);
        }
      }

      children:

      if (nest)
        lev++;

      for (menu* c (m->list); c != nullptr; c = c->next)
        n += report (c, dr, pad, lev);

      return n;
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

      // All the variables we enter are qualified so go straight for the
      // public variable pool.
      //
      auto& vp (rs.var_pool (true /* public */));

      const dir_path& src_root (rs.src_path ());
      const dir_path& out_root (rs.out_path ());

      // Custom main menu title ("<project> <version>" by default).
      //
      auto& var_k_k_t (vp.insert<string> ("kconfig.kconfig.title"));

      // See configure().
      //
      auto& var_k_k_tr (vp.insert<strings> ("kconfig.kconfig.transient"));

      path df (src_root / rs.root_extra->build_dir / def_file);

      if (!exists (df))
        fail (l) << df << " does not exist";

      // (Re)configure if we are configuring.
      //
      path vf (ctx.current_mif->id == configure_id
               ? configure (rs, l, df, var_k_k_t)
               : out_root / rs.root_extra->build_dir / val_file);

      // Note: similar code in configure() above.
      //
      auto conf_g (make_guard ([] () {conf_free ();}));

      // Disable extra diagnostics.
      //
      conf_set_message_callback (nullptr);

      // Resolve getenv() calls as Kconfig.* lookups.
      //
      env_data edata (rs, l, cast_null<string> (rs[var_k_k_t]));
      conf_set_getenv_callback (&build2_kconfig_getenv, &edata);

      // Load the configuration definition (Kconfig).
      //
      conf_parse (env_path (df).c_str ());
      if (edata.failed)
        throw failed ();

      // Load the configuration values.
      //
      bool calc (false);
      if (exists (vf))
      {
        if (conf_read (env_path (vf).c_str ()) != 0)
          fail (l) << "unable to load " << vf;

        if (conf_get_changed ())
          fail (l) << "Kconfig configuration definition/values have changed" <<
            info << "reconfigure to synchronize" <<
            info << "configuration definition: " << df <<
            info << "configuration values: " << vf;
      }
      else
      {
        // Transient configuration.
        //
        const strings* ms (cast_null<strings> (rs[var_k_k_tr]));

        if (ms != nullptr && ms->empty ())
          fail << "configuration method expected in " << var_k_k_tr;

        const string& m (ms != nullptr ? (*ms)[0] : "ask");

        if (m == "ask" || m == "new-ask" || m == "reask" || m == "new-reask")
          fail (l) << vf << " does not exist" <<
            info << "consider configuring " << out_root;

        if (m != "def" && m != "new-def")
          fail << "unexpected configuration method '" << m << "' in "
               << var_k_k_tr;

        switch (ms != nullptr ? ms->size () : 1)
        {
        case 1:
          {
            // --alldefconfig
            //
            conf_set_all_new_symbols (def_default);
            calc = true;
            break;
          }
        case 2:
          {
            // --defconfig <file>
            //
            const string& f ((*ms)[1]);

            // Note: using conf_read_simple() does not work for some reason
            // (even with the below value calculation).
            //
            if (conf_read (env_path (f).c_str ()) != 0)
              fail (l) << "unable to load " << f;

            // Note: appears to return true even for complete configurations
            // created with --savedefconfig.
            //
            calc = conf_set_all_new_symbols (def_default);
            break;
          }
        default:
          {
            fail << "unexpected argument '" << (*ms)[2] << "' for method "
                 << m << " in " << var_k_k_tr;
          }
        }
      }

      const project_name& prj (project (rs));

      // Set Kconfig symbols as kconfig.* variables.
      //
      size_t count (0), pad (10);
      {
        // If this is a named project, we qualify kconfig.* variables with the
        // project and make them global. This qualification has the following
        // benefits:
        //
        // - Consistent with config.*-based project qualification.
        // - Can be overridden on the command line.
        // - Can be used in subprojects.
        //
        bool var_q (!prj.empty ());
        string var_p (var_q ? prj.variable () + '.' : string ());

        // If requested, calculate symbol values, similar to conf_read().
        //
        if (calc)
          sym_calc_value (modules_sym);

        size_t i;
        symbol* s;
        for_all_symbols (i, s)
        {
          if (calc)
            sym_calc_value (s);

          if (!real_symbol (s))
            continue;

          count++;

          // Process the name.
          //
          string var_n ("kconfig." + var_p + s->name);
          lcase (var_n, var_p.size () + 8);

          // Enter variable.
          //
          // @@ The kconfig module could define the tristate type if there is
          //    demand for it. Would be neat to derive it from bool somehow.
          //
          const value_type* var_t (nullptr);
          if (var_q)
          {
            switch (s->type)
            {
            case S_BOOLEAN:  var_t = &value_traits<bool>::value_type;     break;
            case S_TRISTATE: var_t = &value_traits<string>::value_type;   break;
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
              val = (v.tri == yes ? "true" :
                     v.tri == mod ? "module" : "false");
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
              // The NULL semantics is in effect when the option is disabled
              // because one of its dependencies is not satisfied (note that
              // this does not apply to bool and tristate because they are set
              // to false if disabled).
              //
              bool null (!(s->flags & SYMBOL_WRITE) &&
                         !(s->flags & SYMBOL_NO_WRITE));
              if (null)
              {
                val = nullptr;
                continue;
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

          size_t n (strlen (s->name));
          if (n > pad)
            pad = n;
        }
      }

      // Print the report.
      //
      // Use the fact that we are configuring as a proxy for the new
      // configuration.
      //
      if (count != 0 && verb >= (ctx.current_mif->id == configure_id ? 2 : 3))
      {
        diag_record dr (text);
        dr << "kconfig " << prj << '@' << rs;
        size_t n (report (&rootmenu, dr, pad));
        assert (n == count);
      }

      // Environment (see configure() for details).
      //
      config::save_environment (
        rs, {"KCONFIG_PROBABILITY", "KCONFIG_SEED", "KCONFIG_ALLCONFIG"});

      // Make sure Kconfig-related files are part of the distribution. But
      // let's not waste time if we are not distributing.
      //
      if (ctx.current_mif->id == dist_id)
      {
        if (auto* m = rs.find_module<dist::module> (dist::module::name))
        {
          // Top-level Kconfig as well as all the files it sourced.
          //
          for (const ::file* f (file_list); f != nullptr; f = f->next)
          {
            try
            {
              path p (f->name);
              p.normalize (); // Get rid of `..`, etc.

              if (p.relative ())
                fail << "relative Kconfig path: " << p.string ();

              if (!p.sub (src_root))
                fail << "Kconfig path out of source root: " << p.string ();

              m->add_adhoc (p.leaf (src_root));
            }
            catch (const invalid_path&)
            {
              fail << "invalid Kconfig path '" << f->name << "'";
            }
          }

          // Conventionally-named default configuration files.
          //
          m->add_adhoc (rs.root_extra->build_dir / "defconfig*.kconfig");
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

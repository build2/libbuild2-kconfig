#ifndef CONFAPI_H
#define CONFAPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Note that these functions only do the asking. In particular, they don't
   load the definitions (Kconfig) or values (.config) nor save anything. */

void conf_ask ();   /* --oldconfig */
void conf_reask (); /* --oldaskconfig */

#ifdef __cplusplus
}
#endif

#endif /* CONFAPI_H */

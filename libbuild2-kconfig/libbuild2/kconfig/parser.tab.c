/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 2
#define YYMINOR 0
#define YYPATCH 20210808

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#undef YYBTYACC
#define YYBTYACC 0
#define YYDEBUGSTR YYPREFIX "debug"
#define YYPREFIX "yy"

#define YYPURE 0


#define YY_NO_LEAKS 1


#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lkc.h"
#include "internal.h"

static inline void printd(int mask, const char *format, ...)
{
	va_list ap;
	if (cdebug & mask)
	{
		va_start(ap, format);
		vprintf(format, ap);
		va_end(ap);
	}
}

#define PRINTD		0x0001
#define DEBUG_PARSE	0x0002

int cdebug = PRINTD;

static void yyerror(const char *err);
static void zconfprint(const char *err, ...);
static void zconf_error(const char *err, ...);
static bool zconf_endtoken(const char *tokenname,
			   const char *expected_tokenname);

struct symbol *symbol_hash[SYMBOL_HASHSIZE];

struct menu *current_menu, *current_entry;

#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union YYSTYPE
{
	char *string;
	struct symbol *symbol;
	struct expr *expr;
	struct menu *menu;
	enum symbol_type type;
	enum variable_flavor flavor;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

#if !(defined(yylex) || defined(YYSTATE))
int YYLEX_DECL();
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

#ifndef YYDESTRUCT_DECL
#define YYDESTRUCT_DECL() yydestruct(const char *msg, int psymb, YYSTYPE *val)
#endif
#ifndef YYDESTRUCT_CALL
#define YYDESTRUCT_CALL(msg, psymb, val) yydestruct(msg, psymb, val)
#endif

extern int YYPARSE_DECL();

#define T_HELPTEXT 257
#define T_WORD 258
#define T_WORD_QUOTE 259
#define T_BOOL 260
#define T_CHOICE 261
#define T_CLOSE_PAREN 262
#define T_COLON_EQUAL 263
#define T_COMMENT 264
#define T_CONFIG 265
#define T_DEFAULT 266
#define T_DEF_BOOL 267
#define T_DEF_TRISTATE 268
#define T_DEPENDS 269
#define T_ENDCHOICE 270
#define T_ENDIF 271
#define T_ENDMENU 272
#define T_HELP 273
#define T_HEX 274
#define T_IF 275
#define T_IMPLY 276
#define T_INT 277
#define T_MAINMENU 278
#define T_MENU 279
#define T_MENUCONFIG 280
#define T_MODULES 281
#define T_ON 282
#define T_OPEN_PAREN 283
#define T_OPTIONAL 284
#define T_PLUS_EQUAL 285
#define T_PROMPT 286
#define T_RANGE 287
#define T_SELECT 288
#define T_SOURCE 289
#define T_STRING 290
#define T_TRISTATE 291
#define T_VISIBLE 292
#define T_EOL 293
#define T_ASSIGN_VAL 294
#define T_OR 295
#define T_AND 296
#define T_EQUAL 297
#define T_UNEQUAL 298
#define T_LESS 299
#define T_LESS_EQUAL 300
#define T_GREATER 301
#define T_GREATER_EQUAL 302
#define T_NOT 303
#define YYERRCODE 256
typedef int YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,   15,   16,   16,   16,   16,   16,   16,   16,
   16,   16,   16,   16,   25,   25,   25,   25,   25,   27,
   20,   29,   23,   28,   28,   28,   28,   30,   30,   30,
   30,   30,   30,   30,   34,   11,   36,   18,   35,   35,
   35,   35,   37,   37,   37,   37,    3,    3,    3,    3,
    4,    4,    5,    5,    5,    9,   38,   21,   26,   39,
   10,   41,   22,   40,   40,   40,   24,   43,   19,   44,
   44,   45,   32,   31,   42,   33,   33,    8,    8,    8,
    7,    7,    6,    6,    6,    6,    6,    6,    6,    6,
    6,    6,    6,    1,    2,    2,   12,   12,   17,   14,
   14,   14,   13,   13,
};
static const YYINT yylen[] = {                            2,
    2,    1,    3,    0,    2,    2,    2,    2,    2,    2,
    2,    2,    4,    3,    0,    2,    2,    2,    3,    3,
    2,    3,    2,    0,    2,    2,    2,    3,    4,    4,
    4,    4,    5,    2,    3,    2,    1,    3,    0,    2,
    2,    2,    4,    3,    2,    4,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    3,    1,    3,    3,    3,
    2,    1,    3,    0,    2,    2,    3,    3,    2,    0,
    2,    2,    2,    4,    3,    0,    2,    2,    2,    2,
    0,    2,    1,    3,    3,    3,    3,    3,    3,    3,
    2,    3,    3,    1,    1,    1,    0,    1,    4,    1,
    1,    1,    0,    1,
};
static const YYINT yydefred[] = {                         0,
    0,    0,    4,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    4,    4,   15,    5,    6,
    7,    8,    9,   10,   11,   12,   24,   24,   39,   64,
   70,    3,   14,    0,  101,  102,  100,    0,   98,    0,
    0,   94,    0,   96,    0,    0,   95,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   13,  104,    0,   35,   68,   20,    0,   91,    0,    0,
    0,    0,    0,    0,   56,    0,    0,   60,   22,   67,
    0,    0,    0,   57,   58,   62,   63,    0,   37,   15,
   16,   17,   18,   38,   51,   53,   54,   55,    0,    0,
   49,    0,   48,    0,    0,    0,    0,   50,   52,    0,
   47,    0,   25,   26,   27,    0,    0,    0,    0,    0,
   41,   42,   40,    0,   66,   65,   71,   99,   90,   88,
   89,   84,   85,   86,   87,    0,   93,   79,   80,   78,
   19,    0,    0,   72,    0,   34,    0,    0,    0,    0,
    0,    0,   73,    0,   45,    0,    0,    0,    0,   59,
    0,    0,    0,    0,    0,   77,   28,    0,    0,    0,
   44,    0,   75,   74,   32,   29,    0,   31,   30,   46,
   43,   33,
};
#if defined(YYDESTRUCT_CALL) || defined(YYSTYPE_TOSTRING)
static const YYINT yystos[] = {                           0,
  278,  305,  320,  321,  259,  321,  256,  258,  261,  264,
  265,  275,  279,  280,  289,  314,  315,  316,  322,  323,
  324,  325,  326,  327,  328,  329,  332,  334,  339,  344,
  348,  293,  293,  256,  263,  285,  297,  319,  258,  317,
  259,  258,  306,  259,  283,  303,  306,  307,  311,  259,
  306,  259,  321,  321,  330,  333,  333,  340,  345,  349,
  293,  294,  318,  293,  293,  293,  311,  311,  297,  298,
  299,  300,  301,  302,  293,  295,  296,  293,  293,  293,
  270,  271,  272,  313,  343,  313,  346,  256,  313,  314,
  324,  325,  331,  341,  260,  266,  267,  268,  269,  273,
  274,  276,  277,  281,  286,  287,  288,  290,  291,  308,
  309,  310,  335,  336,  337,  350,  266,  284,  286,  309,
  336,  337,  342,  292,  336,  347,  336,  293,  262,  307,
  307,  307,  307,  307,  307,  311,  311,  293,  293,  293,
  293,  330,  282,  293,  306,  293,  259,  307,  306,  259,
  338,  311,  257,  306,  293,  259,  338,  275,  312,  343,
  311,  312,  312,  307,  312,  312,  293,  312,  312,  312,
  293,  311,  293,  293,  293,  293,  312,  293,  293,  293,
  293,  293,
};
#endif /* YYDESTRUCT_CALL || YYSTYPE_TOSTRING */
static const YYINT yydgoto[] = {                          2,
   47,   48,  110,  111,  112,   49,  159,   84,   16,   17,
   18,   40,   63,   38,    3,    4,   19,   20,   21,   22,
   23,   24,   25,   26,   55,   93,   27,   56,   28,  113,
  114,  115,  151,   29,   58,   94,  123,   85,   30,   59,
   87,  126,   31,   60,  116,
};
static const YYINT yysindex[] = {                      -249,
 -233,    0,    0, -132, -262, -132, -244, -238, -206, -204,
 -200, -243, -193, -200, -191,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, -220,    0,    0,    0, -215,    0, -209,
 -203,    0, -201,    0, -243, -243,    0,  -71, -221, -199,
 -196, -195, -194, -194,  -63, -115, -115,  -92, -250, -180,
    0,    0, -190,    0,    0,    0, -242,    0, -235, -235,
 -235, -235, -235, -235,    0, -243, -243,    0,    0,    0,
 -188, -186, -185,    0,    0,    0,    0, -183,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -168, -177,
    0, -200,    0, -176, -160, -235, -200,    0,    0, -157,
    0, -243,    0,    0,    0, -136, -200, -171, -134, -157,
    0,    0,    0, -145,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -165,    0,    0,    0,    0,
    0,  -63, -243,    0, -145,    0, -145, -235, -145, -145,
 -159, -268,    0, -145,    0, -145, -158, -243, -155,    0,
 -213, -154, -153, -145, -151,    0,    0, -149, -147, -144,
    0, -252,    0,    0,    0,    0, -143,    0,    0,    0,
    0,    0,
};
static const YYINT yyrindex[] = {                        61,
    0,    0,    0,  136,    0,  141,    0,    0, -138,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -137,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -245,    0,    0,
    0,    0,    0,    0,    0,    1,   21,  -50, -152,   41,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -133,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -133,
    0,    0,    0, -130,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -230,    0,    0,    0,    0,
    0,    0,    0,    0, -130,    0, -130,    0, -130, -130,
    0, -130,    0, -130,    0, -130,    0,    0,    0,    0,
    0,    0,    0, -130,    0,    0,    0,    0,    0,    0,
    0, -129,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
#if YYBTYACC
static const YYINT yycindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
#endif
static const YYINT yygindex[] = {                         0,
   -6,  -60,    0,  107,    0,  -43,   33,    2,  -55,    0,
    0,    0,    0,    0,    0,   19,    0,    0,  -51,  -49,
    0,    0,    0,    0,   77,    0,    0,  142,    0,    0,
  -21,  111,   59,    0,    0,    0,    0,   42,    0,    0,
    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 350
static const YYINT yytable[] = {                         90,
   21,   67,   68,   91,   43,   92,  158,   51,  130,  131,
  132,  133,  134,  135,   42,   44,   83,   34,   99,  129,
   23,    6,   42,   44,   35,    5,   76,   77,    1,   83,
   32,   92,  136,  137,   53,   54,  121,  125,  127,   45,
   69,  124,   76,   77,   92,  148,   36,   83,   33,   83,
   83,   39,   76,   77,   41,   86,   89,   42,   37,   46,
    4,    7,   92,    8,   92,   50,    9,   52,  152,   10,
   11,   75,   61,   76,   77,   81,   82,   83,   62,  174,
   12,   76,   77,   64,   13,   14,   90,  164,   99,   65,
   91,   66,   92,   78,   15,  145,   79,   80,  147,  161,
  149,  150,  128,   61,  138,   61,  139,  140,   61,  141,
  154,   61,   61,  143,  172,  144,  146,   61,   61,   61,
  153,  155,   61,    7,  156,    8,   61,   61,    9,  158,
   77,   10,   11,  167,  171,    2,   61,  173,  175,  176,
    1,  178,   12,  179,   95,  180,   13,   14,  181,  182,
   96,   97,   98,   99,   97,  103,   15,  100,  101,   76,
  102,  103,   81,   82,  120,  104,  142,   95,  122,   57,
  105,  106,  107,  117,  108,  109,   99,  162,  157,  163,
  100,  165,  166,  160,  168,    0,  169,    0,  170,    0,
    0,  118,   88,  119,    0,    0,  177,    0,  109,    0,
   10,   11,    0,    0,    0,   36,   81,   82,   83,    0,
    0,   12,    0,   36,   36,    0,    0,    0,    0,   36,
   36,   36,    0,    0,   36,   69,   70,   71,   72,   73,
   74,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   21,    0,   21,    0,
    0,   21,    0,    0,   21,   21,    0,    0,    0,    0,
   21,   21,   21,    0,    0,   21,   23,    0,   23,   21,
   21,   23,    0,    0,   23,   23,    0,    0,    0,   21,
   23,   23,   23,    0,    0,   23,   69,    0,   69,   23,
   23,   69,    0,    0,   69,   69,    0,    0,    0,   23,
   69,   69,   69,    0,    0,   69,    4,    0,    4,   69,
   69,    4,    0,    0,    4,    4,    0,    0,    0,   69,
    0,    0,    0,    0,    0,    4,    0,    0,    0,    4,
    4,    0,    0,    0,    0,    0,    0,    0,    0,    4,
};
static const YYINT yycheck[] = {                         55,
    0,   45,   46,   55,   11,   55,  275,   14,   69,   70,
   71,   72,   73,   74,  258,  259,  262,  256,  269,  262,
    0,    3,  258,  259,  263,  259,  295,  296,  278,  275,
  293,  262,   76,   77,   16,   17,   58,   59,   60,  283,
    0,  292,  295,  296,  275,  106,  285,  293,  293,  295,
  296,  258,  295,  296,  259,   54,   55,  258,  297,  303,
    0,  256,  293,  258,  295,  259,  261,  259,  112,  264,
  265,  293,  293,  295,  296,  270,  271,  272,  294,  293,
  275,  295,  296,  293,  279,  280,  142,  148,  269,  293,
  142,  293,  142,  293,  289,  102,  293,  293,  259,  143,
  107,  259,  293,  256,  293,  258,  293,  293,  261,  293,
  117,  264,  265,  282,  158,  293,  293,  270,  271,  272,
  257,  293,  275,  256,  259,  258,  279,  280,  261,  275,
  296,  264,  265,  293,  293,    0,  289,  293,  293,  293,
    0,  293,  275,  293,  260,  293,  279,  280,  293,  293,
  266,  267,  268,  269,  293,  293,  289,  273,  274,  293,
  276,  277,  293,  293,   58,  281,   90,  260,   58,   28,
  286,  287,  288,  266,  290,  291,  269,  145,  120,  147,
  273,  149,  150,  142,  152,   -1,  154,   -1,  156,   -1,
   -1,  284,  256,  286,   -1,   -1,  164,   -1,  291,   -1,
  264,  265,   -1,   -1,   -1,  256,  270,  271,  272,   -1,
   -1,  275,   -1,  264,  265,   -1,   -1,   -1,   -1,  270,
  271,  272,   -1,   -1,  275,  297,  298,  299,  300,  301,
  302,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  256,   -1,  258,   -1,
   -1,  261,   -1,   -1,  264,  265,   -1,   -1,   -1,   -1,
  270,  271,  272,   -1,   -1,  275,  256,   -1,  258,  279,
  280,  261,   -1,   -1,  264,  265,   -1,   -1,   -1,  289,
  270,  271,  272,   -1,   -1,  275,  256,   -1,  258,  279,
  280,  261,   -1,   -1,  264,  265,   -1,   -1,   -1,  289,
  270,  271,  272,   -1,   -1,  275,  256,   -1,  258,  279,
  280,  261,   -1,   -1,  264,  265,   -1,   -1,   -1,  289,
   -1,   -1,   -1,   -1,   -1,  275,   -1,   -1,   -1,  279,
  280,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  289,
};
#if YYBTYACC
static const YYINT yyctable[] = {                        -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
};
#endif
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 1
#endif
#define YYMAXTOKEN 303
#define YYUNDFTOKEN 351
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"$end",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"error","T_HELPTEXT","T_WORD",
"T_WORD_QUOTE","T_BOOL","T_CHOICE","T_CLOSE_PAREN","T_COLON_EQUAL","T_COMMENT",
"T_CONFIG","T_DEFAULT","T_DEF_BOOL","T_DEF_TRISTATE","T_DEPENDS","T_ENDCHOICE",
"T_ENDIF","T_ENDMENU","T_HELP","T_HEX","T_IF","T_IMPLY","T_INT","T_MAINMENU",
"T_MENU","T_MENUCONFIG","T_MODULES","T_ON","T_OPEN_PAREN","T_OPTIONAL",
"T_PLUS_EQUAL","T_PROMPT","T_RANGE","T_SELECT","T_SOURCE","T_STRING",
"T_TRISTATE","T_VISIBLE","T_EOL","T_ASSIGN_VAL","T_OR","T_AND","T_EQUAL",
"T_UNEQUAL","T_LESS","T_LESS_EQUAL","T_GREATER","T_GREATER_EQUAL","T_NOT",
"$accept","input","nonconst_symbol","symbol","type","logic_type","default",
"expr","if_expr","end","if_entry","menu_entry","choice_entry","word_opt",
"assign_val","assign_op","mainmenu_stmt","stmt_list","assignment_stmt",
"choice_stmt","comment_stmt","config_stmt","if_stmt","menu_stmt",
"menuconfig_stmt","source_stmt","stmt_list_in_choice","if_stmt_in_choice",
"config_entry_start","config_option_list","menuconfig_entry_start",
"config_option","depends","help","prompt_stmt_opt","choice",
"choice_option_list","choice_end","choice_option","if_end","menu",
"menu_option_list","menu_end","visible","comment","comment_option_list",
"help_start","illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : input",
"input : mainmenu_stmt stmt_list",
"input : stmt_list",
"mainmenu_stmt : T_MAINMENU T_WORD_QUOTE T_EOL",
"stmt_list :",
"stmt_list : stmt_list assignment_stmt",
"stmt_list : stmt_list choice_stmt",
"stmt_list : stmt_list comment_stmt",
"stmt_list : stmt_list config_stmt",
"stmt_list : stmt_list if_stmt",
"stmt_list : stmt_list menu_stmt",
"stmt_list : stmt_list menuconfig_stmt",
"stmt_list : stmt_list source_stmt",
"stmt_list : stmt_list T_WORD error T_EOL",
"stmt_list : stmt_list error T_EOL",
"stmt_list_in_choice :",
"stmt_list_in_choice : stmt_list_in_choice comment_stmt",
"stmt_list_in_choice : stmt_list_in_choice config_stmt",
"stmt_list_in_choice : stmt_list_in_choice if_stmt_in_choice",
"stmt_list_in_choice : stmt_list_in_choice error T_EOL",
"config_entry_start : T_CONFIG nonconst_symbol T_EOL",
"config_stmt : config_entry_start config_option_list",
"menuconfig_entry_start : T_MENUCONFIG nonconst_symbol T_EOL",
"menuconfig_stmt : menuconfig_entry_start config_option_list",
"config_option_list :",
"config_option_list : config_option_list config_option",
"config_option_list : config_option_list depends",
"config_option_list : config_option_list help",
"config_option : type prompt_stmt_opt T_EOL",
"config_option : T_PROMPT T_WORD_QUOTE if_expr T_EOL",
"config_option : default expr if_expr T_EOL",
"config_option : T_SELECT nonconst_symbol if_expr T_EOL",
"config_option : T_IMPLY nonconst_symbol if_expr T_EOL",
"config_option : T_RANGE symbol symbol if_expr T_EOL",
"config_option : T_MODULES T_EOL",
"choice : T_CHOICE word_opt T_EOL",
"choice_entry : choice choice_option_list",
"choice_end : end",
"choice_stmt : choice_entry stmt_list_in_choice choice_end",
"choice_option_list :",
"choice_option_list : choice_option_list choice_option",
"choice_option_list : choice_option_list depends",
"choice_option_list : choice_option_list help",
"choice_option : T_PROMPT T_WORD_QUOTE if_expr T_EOL",
"choice_option : logic_type prompt_stmt_opt T_EOL",
"choice_option : T_OPTIONAL T_EOL",
"choice_option : T_DEFAULT nonconst_symbol if_expr T_EOL",
"type : logic_type",
"type : T_INT",
"type : T_HEX",
"type : T_STRING",
"logic_type : T_BOOL",
"logic_type : T_TRISTATE",
"default : T_DEFAULT",
"default : T_DEF_BOOL",
"default : T_DEF_TRISTATE",
"if_entry : T_IF expr T_EOL",
"if_end : end",
"if_stmt : if_entry stmt_list if_end",
"if_stmt_in_choice : if_entry stmt_list_in_choice if_end",
"menu : T_MENU T_WORD_QUOTE T_EOL",
"menu_entry : menu menu_option_list",
"menu_end : end",
"menu_stmt : menu_entry stmt_list menu_end",
"menu_option_list :",
"menu_option_list : menu_option_list visible",
"menu_option_list : menu_option_list depends",
"source_stmt : T_SOURCE T_WORD_QUOTE T_EOL",
"comment : T_COMMENT T_WORD_QUOTE T_EOL",
"comment_stmt : comment comment_option_list",
"comment_option_list :",
"comment_option_list : comment_option_list depends",
"help_start : T_HELP T_EOL",
"help : help_start T_HELPTEXT",
"depends : T_DEPENDS T_ON expr T_EOL",
"visible : T_VISIBLE if_expr T_EOL",
"prompt_stmt_opt :",
"prompt_stmt_opt : T_WORD_QUOTE if_expr",
"end : T_ENDMENU T_EOL",
"end : T_ENDCHOICE T_EOL",
"end : T_ENDIF T_EOL",
"if_expr :",
"if_expr : T_IF expr",
"expr : symbol",
"expr : symbol T_LESS symbol",
"expr : symbol T_LESS_EQUAL symbol",
"expr : symbol T_GREATER symbol",
"expr : symbol T_GREATER_EQUAL symbol",
"expr : symbol T_EQUAL symbol",
"expr : symbol T_UNEQUAL symbol",
"expr : T_OPEN_PAREN expr T_CLOSE_PAREN",
"expr : T_NOT expr",
"expr : expr T_OR expr",
"expr : expr T_AND expr",
"nonconst_symbol : T_WORD",
"symbol : nonconst_symbol",
"symbol : T_WORD_QUOTE",
"word_opt :",
"word_opt : T_WORD",
"assignment_stmt : T_WORD assign_op assign_val T_EOL",
"assign_op : T_EQUAL",
"assign_op : T_COLON_EQUAL",
"assign_op : T_PLUS_EQUAL",
"assign_val :",
"assign_val : T_ASSIGN_VAL",

};
#endif

#if YYDEBUG
int      yydebug;
#endif

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;
int      yynerrs;

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
YYLTYPE  yyloc; /* position returned by actions */
YYLTYPE  yylloc; /* position from the lexer */
#endif

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
#ifndef YYLLOC_DEFAULT
#define YYLLOC_DEFAULT(loc, rhs, n) \
do \
{ \
    if (n == 0) \
    { \
        (loc).first_line   = YYRHSLOC(rhs, 0).last_line; \
        (loc).first_column = YYRHSLOC(rhs, 0).last_column; \
        (loc).last_line    = YYRHSLOC(rhs, 0).last_line; \
        (loc).last_column  = YYRHSLOC(rhs, 0).last_column; \
    } \
    else \
    { \
        (loc).first_line   = YYRHSLOC(rhs, 1).first_line; \
        (loc).first_column = YYRHSLOC(rhs, 1).first_column; \
        (loc).last_line    = YYRHSLOC(rhs, n).last_line; \
        (loc).last_column  = YYRHSLOC(rhs, n).last_column; \
    } \
} while (0)
#endif /* YYLLOC_DEFAULT */
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
#if YYBTYACC

#ifndef YYLVQUEUEGROWTH
#define YYLVQUEUEGROWTH 32
#endif
#endif /* YYBTYACC */

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#ifndef YYINITSTACKSIZE
#define YYINITSTACKSIZE 200
#endif

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE  *p_base;
    YYLTYPE  *p_mark;
#endif
} YYSTACKDATA;
#if YYBTYACC

struct YYParseState_s
{
    struct YYParseState_s *save;    /* Previously saved parser state */
    YYSTACKDATA            yystack; /* saved parser stack */
    int                    state;   /* saved parser state */
    int                    errflag; /* saved error recovery status */
    int                    lexeme;  /* saved index of the conflict lexeme in the lexical queue */
    YYINT                  ctry;    /* saved index in yyctable[] for this conflict */
};
typedef struct YYParseState_s YYParseState;
#endif /* YYBTYACC */
/* variables for the parser stack */
static YYSTACKDATA yystack;
#if YYBTYACC

/* Current parser state */
static YYParseState *yyps = 0;

/* yypath != NULL: do the full parse, starting at *yypath parser state. */
static YYParseState *yypath = 0;

/* Base of the lexical value queue */
static YYSTYPE *yylvals = 0;

/* Current position at lexical value queue */
static YYSTYPE *yylvp = 0;

/* End position of lexical value queue */
static YYSTYPE *yylve = 0;

/* The last allocated position at the lexical value queue */
static YYSTYPE *yylvlim = 0;

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
/* Base of the lexical position queue */
static YYLTYPE *yylpsns = 0;

/* Current position at lexical position queue */
static YYLTYPE *yylpp = 0;

/* End position of lexical position queue */
static YYLTYPE *yylpe = 0;

/* The last allocated position at the lexical position queue */
static YYLTYPE *yylplim = 0;
#endif

/* Current position at lexical token queue */
static YYINT  *yylexp = 0;

static YYINT  *yylexemes = 0;
#endif /* YYBTYACC */

void conf_parse(const char *name)
{
	struct symbol *sym;
	int i;

	zconf_initscan(name);

	_menu_init();

	if (conf_getenv("ZCONF_DEBUG"))
		yydebug = 1;
	yyparse();

	/* Variables are expanded in the parse phase. We can free them here. */
	variable_all_del();

	if (yynerrs)
		exit(1);
	if (!modules_sym)
		modules_sym = sym_find( "n" );

	if (!menu_has_prompt(&rootmenu)) {
		char *prompt = conf_getenv("KCONFIG_MAINMENU");
		if (!prompt)
			prompt = "Main menu";
		current_entry = &rootmenu;
		menu_add_prompt(P_MENU, xstrdup(prompt), NULL);
	}

	menu_finalize(&rootmenu);
	for_all_symbols(i, sym) {
		if (sym_check_deps(sym))
			yynerrs++;
	}
	if (yynerrs)
		exit(1);
	conf_set_changed(true);
}

static bool zconf_endtoken(const char *tokenname,
			   const char *expected_tokenname)
{
	if (strcmp(tokenname, expected_tokenname)) {
		zconf_error("unexpected '%s' within %s block",
			    tokenname, expected_tokenname);
		yynerrs++;
		return false;
	}
	if (current_menu->file != current_file) {
		zconf_error("'%s' in different file than '%s'",
			    tokenname, expected_tokenname);
		fprintf(stderr, "  %s:%d: info: location of '%s'\n",
			current_menu->file->name, current_menu->lineno,
			expected_tokenname);
		yynerrs++;
		return false;
	}
	return true;
}

static void zconfprint(const char *err, ...)
{
	va_list ap;

	fprintf(stderr, "%s:%d: ", zconf_curname(), zconf_lineno());
	va_start(ap, err);
	vfprintf(stderr, err, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

static void zconf_error(const char *err, ...)
{
	va_list ap;

	yynerrs++;
	fprintf(stderr, "%s:%d: error: ", zconf_curname(), zconf_lineno());
	va_start(ap, err);
	vfprintf(stderr, err, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

static void yyerror(const char *err)
{
	fprintf(stderr, "%s:%d: error: %s\n", zconf_curname(), zconf_lineno() + 1, err);
}

static void print_quoted_string(FILE *out, const char *str)
{
	const char *p;
	int len;

	putc('"', out);
	while ((p = strchr(str, '"'))) {
		len = p - str;
		if (len)
			fprintf(out, "%.*s", len, str);
		fputs("\\\"", out);
		str = p + 1;
	}
	fputs(str, out);
	putc('"', out);
}

static void print_symbol(FILE *out, struct menu *menu)
{
	struct symbol *sym = menu->sym;
	struct property *prop;

	if (sym_is_choice(sym))
		fprintf(out, "\nchoice\n");
	else
		fprintf(out, "\nconfig %s\n", sym->name);
	switch (sym->type) {
	case S_BOOLEAN:
		fputs("  bool\n", out);
		break;
	case S_TRISTATE:
		fputs("  tristate\n", out);
		break;
	case S_STRING:
		fputs("  string\n", out);
		break;
	case S_INT:
		fputs("  integer\n", out);
		break;
	case S_HEX:
		fputs("  hex\n", out);
		break;
	default:
		fputs("  ???\n", out);
		break;
	}
	for (prop = sym->prop; prop; prop = prop->next) {
		if (prop->menu != menu)
			continue;
		switch (prop->type) {
		case P_PROMPT:
			fputs("  prompt ", out);
			print_quoted_string(out, prop->text);
			if (!expr_is_yes(prop->visible.expr)) {
				fputs(" if ", out);
				expr_fprint(prop->visible.expr, out);
			}
			fputc('\n', out);
			break;
		case P_DEFAULT:
			fputs( "  default ", out);
			expr_fprint(prop->expr, out);
			if (!expr_is_yes(prop->visible.expr)) {
				fputs(" if ", out);
				expr_fprint(prop->visible.expr, out);
			}
			fputc('\n', out);
			break;
		case P_CHOICE:
			fputs("  #choice value\n", out);
			break;
		case P_SELECT:
			fputs( "  select ", out);
			expr_fprint(prop->expr, out);
			fputc('\n', out);
			break;
		case P_IMPLY:
			fputs( "  imply ", out);
			expr_fprint(prop->expr, out);
			fputc('\n', out);
			break;
		case P_RANGE:
			fputs( "  range ", out);
			expr_fprint(prop->expr, out);
			fputc('\n', out);
			break;
		case P_MENU:
			fputs( "  menu ", out);
			print_quoted_string(out, prop->text);
			fputc('\n', out);
			break;
		case P_SYMBOL:
			fputs( "  symbol ", out);
			fprintf(out, "%s\n", prop->menu->sym->name);
			break;
		default:
			fprintf(out, "  unknown prop %d!\n", prop->type);
			break;
		}
	}
	if (menu->help) {
		int len = strlen(menu->help);
		while (menu->help[--len] == '\n')
			menu->help[len] = 0;
		fprintf(out, "  help\n%s\n", menu->help);
	}
}

void zconfdump(FILE *out)
{
	struct property *prop;
	struct symbol *sym;
	struct menu *menu;

	menu = rootmenu.list;
	while (menu) {
		if ((sym = menu->sym))
			print_symbol(out, menu);
		else if ((prop = menu->prompt)) {
			switch (prop->type) {
			case P_COMMENT:
				fputs("\ncomment ", out);
				print_quoted_string(out, prop->text);
				fputs("\n", out);
				break;
			case P_MENU:
				fputs("\nmenu ", out);
				print_quoted_string(out, prop->text);
				fputs("\n", out);
				break;
			default:
				;
			}
			if (!expr_is_yes(prop->visible.expr)) {
				fputs("  depends ", out);
				expr_fprint(prop->visible.expr, out);
				fputc('\n', out);
			}
		}

		if (menu->list)
			menu = menu->list;
		else if (menu->next)
			menu = menu->next;
		else while ((menu = menu->parent)) {
			if (menu->prompt && menu->prompt->type == P_MENU)
				fputs("\nendmenu\n", out);
			if (menu->next) {
				menu = menu->next;
				break;
			}
		}
	}
}

/* Release memory associated with symbol. */
#if ! defined YYDESTRUCT_IS_DECLARED
static void
YYDESTRUCT_DECL()
{
    switch (psymb)
    {
	case 314:
	{
	fprintf(stderr, "%s:%d: error: missing end statement for this entry\n",
		(*val).menu->file->name, (*val).menu->lineno);
	if (current_menu == (*val).menu)
		menu_end_menu();
}
	break;
	case 315:
	{
	fprintf(stderr, "%s:%d: error: missing end statement for this entry\n",
		(*val).menu->file->name, (*val).menu->lineno);
	if (current_menu == (*val).menu)
		menu_end_menu();
}
	break;
	case 316:
	{
	fprintf(stderr, "%s:%d: error: missing end statement for this entry\n",
		(*val).menu->file->name, (*val).menu->lineno);
	if (current_menu == (*val).menu)
		menu_end_menu();
}
	break;
    }
}
#define YYDESTRUCT_IS_DECLARED 1
#endif

/* For use in generated program */
#define yydepth (int)(yystack.s_mark - yystack.s_base)
#if YYBTYACC
#define yytrial (yyps->save)
#endif /* YYBTYACC */

#if YYDEBUG
#include <stdio.h>	/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE *newps;
#endif

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    newps = (YYLTYPE *)realloc(data->p_base, newsize * sizeof(*newps));
    if (newps == 0)
        return YYENOMEM;

    data->p_base = newps;
    data->p_mark = newps + i;
#endif

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;

#if YYDEBUG
    if (yydebug)
        fprintf(stderr, "%sdebug: stack size increased to %d\n", YYPREFIX, newsize);
#endif
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    free(data->p_base);
#endif
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif /* YYPURE || defined(YY_NO_LEAKS) */
#if YYBTYACC

static YYParseState *
yyNewState(unsigned size)
{
    YYParseState *p = (YYParseState *) malloc(sizeof(YYParseState));
    if (p == NULL) return NULL;

    p->yystack.stacksize = size;
    if (size == 0)
    {
        p->yystack.s_base = NULL;
        p->yystack.l_base = NULL;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        p->yystack.p_base = NULL;
#endif
        return p;
    }
    p->yystack.s_base    = (YYINT *) malloc(size * sizeof(YYINT));
    if (p->yystack.s_base == NULL) return NULL;
    p->yystack.l_base    = (YYSTYPE *) malloc(size * sizeof(YYSTYPE));
    if (p->yystack.l_base == NULL) return NULL;
    memset(p->yystack.l_base, 0, size * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    p->yystack.p_base    = (YYLTYPE *) malloc(size * sizeof(YYLTYPE));
    if (p->yystack.p_base == NULL) return NULL;
    memset(p->yystack.p_base, 0, size * sizeof(YYLTYPE));
#endif

    return p;
}

static void
yyFreeState(YYParseState *p)
{
    yyfreestack(&p->yystack);
    free(p);
}
#endif /* YYBTYACC */

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab
#if YYBTYACC
#define YYVALID        do { if (yyps->save)            goto yyvalid; } while(0)
#define YYVALID_NESTED do { if (yyps->save && \
                                yyps->save->save == 0) goto yyvalid; } while(0)
#endif /* YYBTYACC */

int
YYPARSE_DECL()
{
    int yym, yyn, yystate, yyresult;
#if YYBTYACC
    int yynewerrflag;
    YYParseState *yyerrctx = NULL;
#endif /* YYBTYACC */
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    YYLTYPE  yyerror_loc_range[3]; /* position of error start/end (0 unused) */
#endif
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
    if (yydebug)
        fprintf(stderr, "%sdebug[<# of symbols on state stack>]\n", YYPREFIX);
#endif
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    memset(yyerror_loc_range, 0, sizeof(yyerror_loc_range));
#endif

#if YYBTYACC
    yyps = yyNewState(0); if (yyps == 0) goto yyenomem;
    yyps->save = 0;
#endif /* YYBTYACC */
    yym = 0;
    /* yyn is set below */
    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark = yystack.p_base;
#endif
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
#if YYBTYACC
        do {
        if (yylvp < yylve)
        {
            /* we're currently re-reading tokens */
            yylval = *yylvp++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yylloc = *yylpp++;
#endif
            yychar = *yylexp++;
            break;
        }
        if (yyps->save)
        {
            /* in trial mode; save scanner results for future parse attempts */
            if (yylvp == yylvlim)
            {   /* Enlarge lexical value queue */
                size_t p = (size_t) (yylvp - yylvals);
                size_t s = (size_t) (yylvlim - yylvals);

                s += YYLVQUEUEGROWTH;
                if ((yylexemes = (YYINT *)realloc(yylexemes, s * sizeof(YYINT))) == NULL) goto yyenomem;
                if ((yylvals   = (YYSTYPE *)realloc(yylvals, s * sizeof(YYSTYPE))) == NULL) goto yyenomem;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                if ((yylpsns   = (YYLTYPE *)realloc(yylpsns, s * sizeof(YYLTYPE))) == NULL) goto yyenomem;
#endif
                yylvp   = yylve = yylvals + p;
                yylvlim = yylvals + s;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylpp   = yylpe = yylpsns + p;
                yylplim = yylpsns + s;
#endif
                yylexp  = yylexemes + p;
            }
            *yylexp = (YYINT) YYLEX;
            *yylvp++ = yylval;
            yylve++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            *yylpp++ = yylloc;
            yylpe++;
#endif
            yychar = *yylexp++;
            break;
        }
        /* normal operation, no conflict encountered */
#endif /* YYBTYACC */
        yychar = YYLEX;
#if YYBTYACC
        } while (0);
#endif /* YYBTYACC */
        if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            fprintf(stderr, "%s[%d]: state %d, reading token %d (%s)",
                            YYDEBUGSTR, yydepth, yystate, yychar, yys);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
            if (!yytrial)
#endif /* YYBTYACC */
                fprintf(stderr, " <%s>", YYSTYPE_TOSTRING(yychar, yylval));
#endif
            fputc('\n', stderr);
        }
#endif
    }
#if YYBTYACC

    /* Do we have a conflict? */
    if (((yyn = yycindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
        yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        YYINT ctry;

        if (yypath)
        {
            YYParseState *save;
#if YYDEBUG
            if (yydebug)
                fprintf(stderr, "%s[%d]: CONFLICT in state %d: following successful trial parse\n",
                                YYDEBUGSTR, yydepth, yystate);
#endif
            /* Switch to the next conflict context */
            save = yypath;
            yypath = save->save;
            save->save = NULL;
            ctry = save->ctry;
            if (save->state != yystate) YYABORT;
            yyFreeState(save);

        }
        else
        {

            /* Unresolved conflict - start/continue trial parse */
            YYParseState *save;
#if YYDEBUG
            if (yydebug)
            {
                fprintf(stderr, "%s[%d]: CONFLICT in state %d. ", YYDEBUGSTR, yydepth, yystate);
                if (yyps->save)
                    fputs("ALREADY in conflict, continuing trial parse.\n", stderr);
                else
                    fputs("Starting trial parse.\n", stderr);
            }
#endif
            save                  = yyNewState((unsigned)(yystack.s_mark - yystack.s_base + 1));
            if (save == NULL) goto yyenomem;
            save->save            = yyps->save;
            save->state           = yystate;
            save->errflag         = yyerrflag;
            save->yystack.s_mark  = save->yystack.s_base + (yystack.s_mark - yystack.s_base);
            memcpy (save->yystack.s_base, yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
            save->yystack.l_mark  = save->yystack.l_base + (yystack.l_mark - yystack.l_base);
            memcpy (save->yystack.l_base, yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            save->yystack.p_mark  = save->yystack.p_base + (yystack.p_mark - yystack.p_base);
            memcpy (save->yystack.p_base, yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
            ctry                  = yytable[yyn];
            if (yyctable[ctry] == -1)
            {
#if YYDEBUG
                if (yydebug && yychar >= YYEOF)
                    fprintf(stderr, "%s[%d]: backtracking 1 token\n", YYDEBUGSTR, yydepth);
#endif
                ctry++;
            }
            save->ctry = ctry;
            if (yyps->save == NULL)
            {
                /* If this is a first conflict in the stack, start saving lexemes */
                if (!yylexemes)
                {
                    yylexemes = (YYINT *) malloc((YYLVQUEUEGROWTH) * sizeof(YYINT));
                    if (yylexemes == NULL) goto yyenomem;
                    yylvals   = (YYSTYPE *) malloc((YYLVQUEUEGROWTH) * sizeof(YYSTYPE));
                    if (yylvals == NULL) goto yyenomem;
                    yylvlim   = yylvals + YYLVQUEUEGROWTH;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    yylpsns   = (YYLTYPE *) malloc((YYLVQUEUEGROWTH) * sizeof(YYLTYPE));
                    if (yylpsns == NULL) goto yyenomem;
                    yylplim   = yylpsns + YYLVQUEUEGROWTH;
#endif
                }
                if (yylvp == yylve)
                {
                    yylvp  = yylve = yylvals;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    yylpp  = yylpe = yylpsns;
#endif
                    yylexp = yylexemes;
                    if (yychar >= YYEOF)
                    {
                        *yylve++ = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                        *yylpe++ = yylloc;
#endif
                        *yylexp  = (YYINT) yychar;
                        yychar   = YYEMPTY;
                    }
                }
            }
            if (yychar >= YYEOF)
            {
                yylvp--;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylpp--;
#endif
                yylexp--;
                yychar = YYEMPTY;
            }
            save->lexeme = (int) (yylvp - yylvals);
            yyps->save   = save;
        }
        if (yytable[yyn] == ctry)
        {
#if YYDEBUG
            if (yydebug)
                fprintf(stderr, "%s[%d]: state %d, shifting to state %d\n",
                                YYDEBUGSTR, yydepth, yystate, yyctable[ctry]);
#endif
            if (yychar < 0)
            {
                yylvp++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylpp++;
#endif
                yylexp++;
            }
            if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM)
                goto yyoverflow;
            yystate = yyctable[ctry];
            *++yystack.s_mark = (YYINT) yystate;
            *++yystack.l_mark = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            *++yystack.p_mark = yylloc;
#endif
            yychar  = YYEMPTY;
            if (yyerrflag > 0) --yyerrflag;
            goto yyloop;
        }
        else
        {
            yyn = yyctable[ctry];
            goto yyreduce;
        }
    } /* End of code dealing with conflicts */
#endif /* YYBTYACC */
    if (((yyn = yysindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
#if YYDEBUG
        if (yydebug)
            fprintf(stderr, "%s[%d]: state %d, shifting to state %d\n",
                            YYDEBUGSTR, yydepth, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        *++yystack.p_mark = yylloc;
#endif
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if (((yyn = yyrindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag != 0) goto yyinrecovery;
#if YYBTYACC

    yynewerrflag = 1;
    goto yyerrhandler;
    goto yyerrlab; /* redundant goto avoids 'unused label' warning */

yyerrlab:
    /* explicit YYERROR from an action -- pop the rhs of the rule reduced
     * before looking for error recovery */
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark -= yym;
#endif

    yynewerrflag = 0;
yyerrhandler:
    while (yyps->save)
    {
        int ctry;
        YYParseState *save = yyps->save;
#if YYDEBUG
        if (yydebug)
            fprintf(stderr, "%s[%d]: ERROR in state %d, CONFLICT BACKTRACKING to state %d, %d tokens\n",
                            YYDEBUGSTR, yydepth, yystate, yyps->save->state,
                    (int)(yylvp - yylvals - yyps->save->lexeme));
#endif
        /* Memorize most forward-looking error state in case it's really an error. */
        if (yyerrctx == NULL || yyerrctx->lexeme < yylvp - yylvals)
        {
            /* Free old saved error context state */
            if (yyerrctx) yyFreeState(yyerrctx);
            /* Create and fill out new saved error context state */
            yyerrctx                 = yyNewState((unsigned)(yystack.s_mark - yystack.s_base + 1));
            if (yyerrctx == NULL) goto yyenomem;
            yyerrctx->save           = yyps->save;
            yyerrctx->state          = yystate;
            yyerrctx->errflag        = yyerrflag;
            yyerrctx->yystack.s_mark = yyerrctx->yystack.s_base + (yystack.s_mark - yystack.s_base);
            memcpy (yyerrctx->yystack.s_base, yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
            yyerrctx->yystack.l_mark = yyerrctx->yystack.l_base + (yystack.l_mark - yystack.l_base);
            memcpy (yyerrctx->yystack.l_base, yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yyerrctx->yystack.p_mark = yyerrctx->yystack.p_base + (yystack.p_mark - yystack.p_base);
            memcpy (yyerrctx->yystack.p_base, yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
            yyerrctx->lexeme         = (int) (yylvp - yylvals);
        }
        yylvp          = yylvals   + save->lexeme;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        yylpp          = yylpsns   + save->lexeme;
#endif
        yylexp         = yylexemes + save->lexeme;
        yychar         = YYEMPTY;
        yystack.s_mark = yystack.s_base + (save->yystack.s_mark - save->yystack.s_base);
        memcpy (yystack.s_base, save->yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
        yystack.l_mark = yystack.l_base + (save->yystack.l_mark - save->yystack.l_base);
        memcpy (yystack.l_base, save->yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        yystack.p_mark = yystack.p_base + (save->yystack.p_mark - save->yystack.p_base);
        memcpy (yystack.p_base, save->yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
        ctry           = ++save->ctry;
        yystate        = save->state;
        /* We tried shift, try reduce now */
        if ((yyn = yyctable[ctry]) >= 0) goto yyreduce;
        yyps->save     = save->save;
        save->save     = NULL;
        yyFreeState(save);

        /* Nothing left on the stack -- error */
        if (!yyps->save)
        {
#if YYDEBUG
            if (yydebug)
                fprintf(stderr, "%sdebug[%d,trial]: trial parse FAILED, entering ERROR mode\n",
                                YYPREFIX, yydepth);
#endif
            /* Restore state as it was in the most forward-advanced error */
            yylvp          = yylvals   + yyerrctx->lexeme;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yylpp          = yylpsns   + yyerrctx->lexeme;
#endif
            yylexp         = yylexemes + yyerrctx->lexeme;
            yychar         = yylexp[-1];
            yylval         = yylvp[-1];
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yylloc         = yylpp[-1];
#endif
            yystack.s_mark = yystack.s_base + (yyerrctx->yystack.s_mark - yyerrctx->yystack.s_base);
            memcpy (yystack.s_base, yyerrctx->yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
            yystack.l_mark = yystack.l_base + (yyerrctx->yystack.l_mark - yyerrctx->yystack.l_base);
            memcpy (yystack.l_base, yyerrctx->yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            yystack.p_mark = yystack.p_base + (yyerrctx->yystack.p_mark - yyerrctx->yystack.p_base);
            memcpy (yystack.p_base, yyerrctx->yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
            yystate        = yyerrctx->state;
            yyFreeState(yyerrctx);
            yyerrctx       = NULL;
        }
        yynewerrflag = 1;
    }
    if (yynewerrflag == 0) goto yyinrecovery;
#endif /* YYBTYACC */

    YYERROR_CALL("syntax error");
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yyerror_loc_range[1] = yylloc; /* lookahead position is error start position */
#endif

#if !YYBTYACC
    goto yyerrlab; /* redundant goto avoids 'unused label' warning */
yyerrlab:
#endif
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if (((yyn = yysindex[*yystack.s_mark]) != 0) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    fprintf(stderr, "%s[%d]: state %d, error recovery shifting to state %d\n",
                                    YYDEBUGSTR, yydepth, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                /* lookahead position is error end position */
                yyerror_loc_range[2] = yylloc;
                YYLLOC_DEFAULT(yyloc, yyerror_loc_range, 2); /* position of error span */
                *++yystack.p_mark = yyloc;
#endif
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    fprintf(stderr, "%s[%d]: error recovery discarding state %d\n",
                                    YYDEBUGSTR, yydepth, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                /* the current TOS position is the error start position */
                yyerror_loc_range[1] = *yystack.p_mark;
#endif
#if defined(YYDESTRUCT_CALL)
#if YYBTYACC
                if (!yytrial)
#endif /* YYBTYACC */
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    YYDESTRUCT_CALL("error: discarding state",
                                    yystos[*yystack.s_mark], yystack.l_mark, yystack.p_mark);
#else
                    YYDESTRUCT_CALL("error: discarding state",
                                    yystos[*yystack.s_mark], yystack.l_mark);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
#endif /* defined(YYDESTRUCT_CALL) */
                --yystack.s_mark;
                --yystack.l_mark;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                --yystack.p_mark;
#endif
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            fprintf(stderr, "%s[%d]: state %d, error recovery discarding token %d (%s)\n",
                            YYDEBUGSTR, yydepth, yystate, yychar, yys);
        }
#endif
#if defined(YYDESTRUCT_CALL)
#if YYBTYACC
        if (!yytrial)
#endif /* YYBTYACC */
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
            YYDESTRUCT_CALL("error: discarding token", yychar, &yylval, &yylloc);
#else
            YYDESTRUCT_CALL("error: discarding token", yychar, &yylval);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
#endif /* defined(YYDESTRUCT_CALL) */
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
    yym = yylen[yyn];
#if YYDEBUG
    if (yydebug)
    {
        fprintf(stderr, "%s[%d]: state %d, reducing by rule %d (%s)",
                        YYDEBUGSTR, yydepth, yystate, yyn, yyrule[yyn]);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
        if (!yytrial)
#endif /* YYBTYACC */
            if (yym > 0)
            {
                int i;
                fputc('<', stderr);
                for (i = yym; i > 0; i--)
                {
                    if (i != yym) fputs(", ", stderr);
                    fputs(YYSTYPE_TOSTRING(yystos[yystack.s_mark[1-i]],
                                           yystack.l_mark[1-i]), stderr);
                }
                fputc('>', stderr);
            }
#endif
        fputc('\n', stderr);
    }
#endif
    if (yym > 0)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)

    /* Perform position reduction */
    memset(&yyloc, 0, sizeof(yyloc));
#if YYBTYACC
    if (!yytrial)
#endif /* YYBTYACC */
    {
        YYLLOC_DEFAULT(yyloc, &yystack.p_mark[-yym], yym);
        /* just in case YYERROR is invoked within the action, save
           the start of the rhs as the error start position */
        yyerror_loc_range[1] = yystack.p_mark[1-yym];
    }
#endif

    switch (yyn)
    {
case 3:
	{
	menu_add_prompt(P_MENU, yystack.l_mark[-1].string, NULL);
}
break;
case 13:
	{ zconf_error("unknown statement \"%s\"", yystack.l_mark[-2].string); }
break;
case 14:
	{ zconf_error("invalid statement"); }
break;
case 19:
	{ zconf_error("invalid statement"); }
break;
case 20:
	{
	yystack.l_mark[-1].symbol->flags |= SYMBOL_OPTIONAL;
	menu_add_entry(yystack.l_mark[-1].symbol);
	printd(DEBUG_PARSE, "%s:%d:config %s\n", zconf_curname(), zconf_lineno(), yystack.l_mark[-1].symbol->name);
}
break;
case 21:
	{
	printd(DEBUG_PARSE, "%s:%d:endconfig\n", zconf_curname(), zconf_lineno());
}
break;
case 22:
	{
	yystack.l_mark[-1].symbol->flags |= SYMBOL_OPTIONAL;
	menu_add_entry(yystack.l_mark[-1].symbol);
	printd(DEBUG_PARSE, "%s:%d:menuconfig %s\n", zconf_curname(), zconf_lineno(), yystack.l_mark[-1].symbol->name);
}
break;
case 23:
	{
	if (current_entry->prompt)
		current_entry->prompt->type = P_MENU;
	else
		zconfprint("warning: menuconfig statement without prompt");
	printd(DEBUG_PARSE, "%s:%d:endconfig\n", zconf_curname(), zconf_lineno());
}
break;
case 28:
	{
	menu_set_type(yystack.l_mark[-2].type);
	printd(DEBUG_PARSE, "%s:%d:type(%u)\n",
		zconf_curname(), zconf_lineno(),
		yystack.l_mark[-2].type);
}
break;
case 29:
	{
	menu_add_prompt(P_PROMPT, yystack.l_mark[-2].string, yystack.l_mark[-1].expr);
	printd(DEBUG_PARSE, "%s:%d:prompt\n", zconf_curname(), zconf_lineno());
}
break;
case 30:
	{
	menu_add_expr(P_DEFAULT, yystack.l_mark[-2].expr, yystack.l_mark[-1].expr);
	if (yystack.l_mark[-3].type != S_UNKNOWN)
		menu_set_type(yystack.l_mark[-3].type);
	printd(DEBUG_PARSE, "%s:%d:default(%u)\n",
		zconf_curname(), zconf_lineno(),
		yystack.l_mark[-3].type);
}
break;
case 31:
	{
	menu_add_symbol(P_SELECT, yystack.l_mark[-2].symbol, yystack.l_mark[-1].expr);
	printd(DEBUG_PARSE, "%s:%d:select\n", zconf_curname(), zconf_lineno());
}
break;
case 32:
	{
	menu_add_symbol(P_IMPLY, yystack.l_mark[-2].symbol, yystack.l_mark[-1].expr);
	printd(DEBUG_PARSE, "%s:%d:imply\n", zconf_curname(), zconf_lineno());
}
break;
case 33:
	{
	menu_add_expr(P_RANGE, expr_alloc_comp(E_RANGE,yystack.l_mark[-3].symbol, yystack.l_mark[-2].symbol), yystack.l_mark[-1].expr);
	printd(DEBUG_PARSE, "%s:%d:range\n", zconf_curname(), zconf_lineno());
}
break;
case 34:
	{
	if (modules_sym)
		zconf_error("symbol '%s' redefines option 'modules' already defined by symbol '%s'",
			    current_entry->sym->name, modules_sym->name);
	modules_sym = current_entry->sym;
}
break;
case 35:
	{
	struct symbol *sym = sym_lookup(yystack.l_mark[-1].string, SYMBOL_CHOICE);
	sym->flags |= SYMBOL_NO_WRITE;
	menu_add_entry(sym);
	menu_add_expr(P_CHOICE, NULL, NULL);
	free(yystack.l_mark[-1].string);
	printd(DEBUG_PARSE, "%s:%d:choice\n", zconf_curname(), zconf_lineno());
}
break;
case 36:
	{
	yyval.menu = menu_add_menu();
}
break;
case 37:
	{
	if (zconf_endtoken(yystack.l_mark[0].string, "choice")) {
		menu_end_menu();
		printd(DEBUG_PARSE, "%s:%d:endchoice\n", zconf_curname(), zconf_lineno());
	}
}
break;
case 43:
	{
	menu_add_prompt(P_PROMPT, yystack.l_mark[-2].string, yystack.l_mark[-1].expr);
	printd(DEBUG_PARSE, "%s:%d:prompt\n", zconf_curname(), zconf_lineno());
}
break;
case 44:
	{
	menu_set_type(yystack.l_mark[-2].type);
	printd(DEBUG_PARSE, "%s:%d:type(%u)\n",
	       zconf_curname(), zconf_lineno(), yystack.l_mark[-2].type);
}
break;
case 45:
	{
	current_entry->sym->flags |= SYMBOL_OPTIONAL;
	printd(DEBUG_PARSE, "%s:%d:optional\n", zconf_curname(), zconf_lineno());
}
break;
case 46:
	{
	menu_add_symbol(P_DEFAULT, yystack.l_mark[-2].symbol, yystack.l_mark[-1].expr);
	printd(DEBUG_PARSE, "%s:%d:default\n",
	       zconf_curname(), zconf_lineno());
}
break;
case 48:
	{ yyval.type = S_INT; }
break;
case 49:
	{ yyval.type = S_HEX; }
break;
case 50:
	{ yyval.type = S_STRING; }
break;
case 51:
	{ yyval.type = S_BOOLEAN; }
break;
case 52:
	{ yyval.type = S_TRISTATE; }
break;
case 53:
	{ yyval.type = S_UNKNOWN; }
break;
case 54:
	{ yyval.type = S_BOOLEAN; }
break;
case 55:
	{ yyval.type = S_TRISTATE; }
break;
case 56:
	{
	printd(DEBUG_PARSE, "%s:%d:if\n", zconf_curname(), zconf_lineno());
	menu_add_entry(NULL);
	menu_add_dep(yystack.l_mark[-1].expr);
	yyval.menu = menu_add_menu();
}
break;
case 57:
	{
	if (zconf_endtoken(yystack.l_mark[0].string, "if")) {
		menu_end_menu();
		printd(DEBUG_PARSE, "%s:%d:endif\n", zconf_curname(), zconf_lineno());
	}
}
break;
case 60:
	{
	menu_add_entry(NULL);
	menu_add_prompt(P_MENU, yystack.l_mark[-1].string, NULL);
	printd(DEBUG_PARSE, "%s:%d:menu\n", zconf_curname(), zconf_lineno());
}
break;
case 61:
	{
	yyval.menu = menu_add_menu();
}
break;
case 62:
	{
	if (zconf_endtoken(yystack.l_mark[0].string, "menu")) {
		menu_end_menu();
		printd(DEBUG_PARSE, "%s:%d:endmenu\n", zconf_curname(), zconf_lineno());
	}
}
break;
case 67:
	{
	printd(DEBUG_PARSE, "%s:%d:source %s\n", zconf_curname(), zconf_lineno(), yystack.l_mark[-1].string);
	zconf_nextfile(yystack.l_mark[-1].string);
	free(yystack.l_mark[-1].string);
}
break;
case 68:
	{
	menu_add_entry(NULL);
	menu_add_prompt(P_COMMENT, yystack.l_mark[-1].string, NULL);
	printd(DEBUG_PARSE, "%s:%d:comment\n", zconf_curname(), zconf_lineno());
}
break;
case 72:
	{
	printd(DEBUG_PARSE, "%s:%d:help\n", zconf_curname(), zconf_lineno());
	zconf_starthelp();
}
break;
case 73:
	{
	if (current_entry->help) {
		free(current_entry->help);
		zconfprint("warning: '%s' defined with more than one help text -- only the last one will be used",
			   current_entry->sym->name ? current_entry->sym->name : "<choice>");
	}

	/* Is the help text empty or all whitespace? */
	if (yystack.l_mark[0].string[strspn(yystack.l_mark[0].string, " \f\n\r\t\v")] == '\0')
		zconfprint("warning: '%s' defined with blank help text",
			   current_entry->sym->name ? current_entry->sym->name : "<choice>");

	current_entry->help = yystack.l_mark[0].string;
}
break;
case 74:
	{
	menu_add_dep(yystack.l_mark[-1].expr);
	printd(DEBUG_PARSE, "%s:%d:depends on\n", zconf_curname(), zconf_lineno());
}
break;
case 75:
	{
	menu_add_visibility(yystack.l_mark[-1].expr);
}
break;
case 77:
	{
	menu_add_prompt(P_PROMPT, yystack.l_mark[-1].string, yystack.l_mark[0].expr);
}
break;
case 78:
	{ yyval.string = "menu"; }
break;
case 79:
	{ yyval.string = "choice"; }
break;
case 80:
	{ yyval.string = "if"; }
break;
case 81:
	{ yyval.expr = NULL; }
break;
case 82:
	{ yyval.expr = yystack.l_mark[0].expr; }
break;
case 83:
	{ yyval.expr = expr_alloc_symbol(yystack.l_mark[0].symbol); }
break;
case 84:
	{ yyval.expr = expr_alloc_comp(E_LTH, yystack.l_mark[-2].symbol, yystack.l_mark[0].symbol); }
break;
case 85:
	{ yyval.expr = expr_alloc_comp(E_LEQ, yystack.l_mark[-2].symbol, yystack.l_mark[0].symbol); }
break;
case 86:
	{ yyval.expr = expr_alloc_comp(E_GTH, yystack.l_mark[-2].symbol, yystack.l_mark[0].symbol); }
break;
case 87:
	{ yyval.expr = expr_alloc_comp(E_GEQ, yystack.l_mark[-2].symbol, yystack.l_mark[0].symbol); }
break;
case 88:
	{ yyval.expr = expr_alloc_comp(E_EQUAL, yystack.l_mark[-2].symbol, yystack.l_mark[0].symbol); }
break;
case 89:
	{ yyval.expr = expr_alloc_comp(E_UNEQUAL, yystack.l_mark[-2].symbol, yystack.l_mark[0].symbol); }
break;
case 90:
	{ yyval.expr = yystack.l_mark[-1].expr; }
break;
case 91:
	{ yyval.expr = expr_alloc_one(E_NOT, yystack.l_mark[0].expr); }
break;
case 92:
	{ yyval.expr = expr_alloc_two(E_OR, yystack.l_mark[-2].expr, yystack.l_mark[0].expr); }
break;
case 93:
	{ yyval.expr = expr_alloc_two(E_AND, yystack.l_mark[-2].expr, yystack.l_mark[0].expr); }
break;
case 94:
	{ yyval.symbol = sym_lookup(yystack.l_mark[0].string, 0); free(yystack.l_mark[0].string); }
break;
case 96:
	{ yyval.symbol = sym_lookup(yystack.l_mark[0].string, SYMBOL_CONST); free(yystack.l_mark[0].string); }
break;
case 97:
	{ yyval.string = NULL; }
break;
case 99:
	{ variable_add(yystack.l_mark[-3].string, yystack.l_mark[-1].string, yystack.l_mark[-2].flavor); free(yystack.l_mark[-3].string); free(yystack.l_mark[-1].string); }
break;
case 100:
	{ yyval.flavor = VAR_RECURSIVE; }
break;
case 101:
	{ yyval.flavor = VAR_SIMPLE; }
break;
case 102:
	{ yyval.flavor = VAR_APPEND; }
break;
case 103:
	{ yyval.string = xstrdup(""); }
break;
    default:
        break;
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark -= yym;
#endif
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
        {
            fprintf(stderr, "%s[%d]: after reduction, ", YYDEBUGSTR, yydepth);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
            if (!yytrial)
#endif /* YYBTYACC */
                fprintf(stderr, "result is <%s>, ", YYSTYPE_TOSTRING(yystos[YYFINAL], yyval));
#endif
            fprintf(stderr, "shifting from state 0 to final state %d\n", YYFINAL);
        }
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        *++yystack.p_mark = yyloc;
#endif
        if (yychar < 0)
        {
#if YYBTYACC
            do {
            if (yylvp < yylve)
            {
                /* we're currently re-reading tokens */
                yylval = *yylvp++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                yylloc = *yylpp++;
#endif
                yychar = *yylexp++;
                break;
            }
            if (yyps->save)
            {
                /* in trial mode; save scanner results for future parse attempts */
                if (yylvp == yylvlim)
                {   /* Enlarge lexical value queue */
                    size_t p = (size_t) (yylvp - yylvals);
                    size_t s = (size_t) (yylvlim - yylvals);

                    s += YYLVQUEUEGROWTH;
                    if ((yylexemes = (YYINT *)realloc(yylexemes, s * sizeof(YYINT))) == NULL)
                        goto yyenomem;
                    if ((yylvals   = (YYSTYPE *)realloc(yylvals, s * sizeof(YYSTYPE))) == NULL)
                        goto yyenomem;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    if ((yylpsns   = (YYLTYPE *)realloc(yylpsns, s * sizeof(YYLTYPE))) == NULL)
                        goto yyenomem;
#endif
                    yylvp   = yylve = yylvals + p;
                    yylvlim = yylvals + s;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                    yylpp   = yylpe = yylpsns + p;
                    yylplim = yylpsns + s;
#endif
                    yylexp  = yylexemes + p;
                }
                *yylexp = (YYINT) YYLEX;
                *yylvp++ = yylval;
                yylve++;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
                *yylpp++ = yylloc;
                yylpe++;
#endif
                yychar = *yylexp++;
                break;
            }
            /* normal operation, no conflict encountered */
#endif /* YYBTYACC */
            yychar = YYLEX;
#if YYBTYACC
            } while (0);
#endif /* YYBTYACC */
            if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
                fprintf(stderr, "%s[%d]: state %d, reading token %d (%s)\n",
                                YYDEBUGSTR, yydepth, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if (((yyn = yygindex[yym]) != 0) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
    {
        fprintf(stderr, "%s[%d]: after reduction, ", YYDEBUGSTR, yydepth);
#ifdef YYSTYPE_TOSTRING
#if YYBTYACC
        if (!yytrial)
#endif /* YYBTYACC */
            fprintf(stderr, "result is <%s>, ", YYSTYPE_TOSTRING(yystos[yystate], yyval));
#endif
        fprintf(stderr, "shifting from state %d to state %d\n", *yystack.s_mark, yystate);
    }
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    *++yystack.p_mark = yyloc;
#endif
    goto yyloop;
#if YYBTYACC

    /* Reduction declares that this path is valid. Set yypath and do a full parse */
yyvalid:
    if (yypath) YYABORT;
    while (yyps->save)
    {
        YYParseState *save = yyps->save;
        yyps->save = save->save;
        save->save = yypath;
        yypath = save;
    }
#if YYDEBUG
    if (yydebug)
        fprintf(stderr, "%s[%d]: state %d, CONFLICT trial successful, backtracking to state %d, %d tokens\n",
                        YYDEBUGSTR, yydepth, yystate, yypath->state, (int)(yylvp - yylvals - yypath->lexeme));
#endif
    if (yyerrctx)
    {
        yyFreeState(yyerrctx);
        yyerrctx = NULL;
    }
    yylvp          = yylvals + yypath->lexeme;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yylpp          = yylpsns + yypath->lexeme;
#endif
    yylexp         = yylexemes + yypath->lexeme;
    yychar         = YYEMPTY;
    yystack.s_mark = yystack.s_base + (yypath->yystack.s_mark - yypath->yystack.s_base);
    memcpy (yystack.s_base, yypath->yystack.s_base, (size_t) (yystack.s_mark - yystack.s_base + 1) * sizeof(YYINT));
    yystack.l_mark = yystack.l_base + (yypath->yystack.l_mark - yypath->yystack.l_base);
    memcpy (yystack.l_base, yypath->yystack.l_base, (size_t) (yystack.l_mark - yystack.l_base + 1) * sizeof(YYSTYPE));
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
    yystack.p_mark = yystack.p_base + (yypath->yystack.p_mark - yypath->yystack.p_base);
    memcpy (yystack.p_base, yypath->yystack.p_base, (size_t) (yystack.p_mark - yystack.p_base + 1) * sizeof(YYLTYPE));
#endif
    yystate        = yypath->state;
    goto yyloop;
#endif /* YYBTYACC */

yyoverflow:
    YYERROR_CALL("yacc stack overflow");
#if YYBTYACC
    goto yyabort_nomem;
yyenomem:
    YYERROR_CALL("memory exhausted");
yyabort_nomem:
#endif /* YYBTYACC */
    yyresult = 2;
    goto yyreturn;

yyabort:
    yyresult = 1;
    goto yyreturn;

yyaccept:
#if YYBTYACC
    if (yyps->save) goto yyvalid;
#endif /* YYBTYACC */
    yyresult = 0;

yyreturn:
#if defined(YYDESTRUCT_CALL)
    if (yychar != YYEOF && yychar != YYEMPTY)
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        YYDESTRUCT_CALL("cleanup: discarding token", yychar, &yylval, &yylloc);
#else
        YYDESTRUCT_CALL("cleanup: discarding token", yychar, &yylval);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */

    {
        YYSTYPE *pv;
#if defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED)
        YYLTYPE *pp;

        for (pv = yystack.l_base, pp = yystack.p_base; pv <= yystack.l_mark; ++pv, ++pp)
             YYDESTRUCT_CALL("cleanup: discarding state",
                             yystos[*(yystack.s_base + (pv - yystack.l_base))], pv, pp);
#else
        for (pv = yystack.l_base; pv <= yystack.l_mark; ++pv)
             YYDESTRUCT_CALL("cleanup: discarding state",
                             yystos[*(yystack.s_base + (pv - yystack.l_base))], pv);
#endif /* defined(YYLTYPE) || defined(YYLTYPE_IS_DECLARED) */
    }
#endif /* defined(YYDESTRUCT_CALL) */

#if YYBTYACC
    if (yyerrctx)
    {
        yyFreeState(yyerrctx);
        yyerrctx = NULL;
    }
    while (yyps)
    {
        YYParseState *save = yyps;
        yyps = save->save;
        save->save = NULL;
        yyFreeState(save);
    }
    while (yypath)
    {
        YYParseState *save = yypath;
        yypath = save->save;
        save->save = NULL;
        yyFreeState(save);
    }
#endif /* YYBTYACC */
    yyfreestack(&yystack);
    return (yyresult);
}

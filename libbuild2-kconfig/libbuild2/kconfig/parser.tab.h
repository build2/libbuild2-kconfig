#ifndef _yy_defines_h_
#define _yy_defines_h_

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
extern YYSTYPE yylval;

#endif /* _yy_defines_h_ */

#ifndef _yy_defines_h_
#define _yy_defines_h_

#define T_HELPTEXT 257
#define T_WORD 258
#define T_WORD_QUOTE 259
#define T_ALLNOCONFIG_Y 260
#define T_BOOL 261
#define T_CHOICE 262
#define T_CLOSE_PAREN 263
#define T_COLON_EQUAL 264
#define T_COMMENT 265
#define T_CONFIG 266
#define T_DEFAULT 267
#define T_DEFCONFIG_LIST 268
#define T_DEF_BOOL 269
#define T_DEF_TRISTATE 270
#define T_DEPENDS 271
#define T_ENDCHOICE 272
#define T_ENDIF 273
#define T_ENDMENU 274
#define T_HELP 275
#define T_HEX 276
#define T_IF 277
#define T_IMPLY 278
#define T_INT 279
#define T_MAINMENU 280
#define T_MENU 281
#define T_MENUCONFIG 282
#define T_MODULES 283
#define T_ON 284
#define T_OPEN_PAREN 285
#define T_OPTION 286
#define T_OPTIONAL 287
#define T_PLUS_EQUAL 288
#define T_PROMPT 289
#define T_RANGE 290
#define T_SELECT 291
#define T_SOURCE 292
#define T_STRING 293
#define T_TRISTATE 294
#define T_VISIBLE 295
#define T_EOL 296
#define T_ASSIGN_VAL 297
#define T_OR 298
#define T_AND 299
#define T_EQUAL 300
#define T_UNEQUAL 301
#define T_LESS 302
#define T_LESS_EQUAL 303
#define T_GREATER 304
#define T_GREATER_EQUAL 305
#define T_NOT 306
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

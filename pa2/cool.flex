/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;
bool eof = false;

void reset_state(){
    memset(string_buf, 0, sizeof(string_buf));
    string_buf_ptr = string_buf;
    BEGIN 0;
}

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */
int add_string(char *s, bool more);

%}

/*
 * Define names for regular expressions here.
 */

DARROW          =>
CHAR            [^\n\"]
%STATE          COMMENT STRING

%%

 /*
  *  Nested comments
  */


<COMMENT>\*\)               {BEGIN 0;}
<COMMENT>[^*]*              { }
<COMMENT>\(\*               { }
<COMMENT>\*                 { }
<COMMENT><<EOF>>            {if (!eof){cool_yylval.error_msg = "EOF in comment";eof=true;return ERROR;}else{return 0;}}
\(\*                        {BEGIN COMMENT;}

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
<STRING>{CHAR}*\"   { int code = add_string(yytext, false); if(code) return code; }
<STRING>{CHAR}*\n   { int code = add_string(yytext, true); if (code) return code; }
\"                  { string_buf_ptr = string_buf; BEGIN STRING; }



 /*
  *  The multiple-character operators.
  */
\+                       { return '+';  }
\*                       { return '*';  }
\-                       { return '-';  }
\~                       { return '~';  }
\/                       { return '/';  }
\;                       { return ';';  }
\(                       { return '(';  }
\)                       { return ')';  }
\{                       { return '{';  }
\}                       { return '}';  }
\:                       { return ':';  }
\.                       { return '.';  }
\@                       { return '@';  }
\<                       { return '<';  }
\=                       { return '=';  }
\,                       { return ',';  }

true                    { cool_yylval.boolean = true ; return BOOL_CONST;}
false                   { cool_yylval.boolean = false; return BOOL_CONST;}
[A-Z]([A-Za-z_])+       { cool_yylval.symbol = stringtable.add_string(yytext); return TYPEID;}
[0-9]+                  { cool_yylval.symbol = inttable.add_string(yytext); return INT_CONST;}

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */

[C|c][l|L][a|A][S|s][S|s]                { return CLASS; }
[E|e][l|L][s|S][e|E]                     { return ELSE;  }
[F|f][I|i]                               { return FI; }
[I|i][f|F]                               { return IF; }
[i|I][N|n]                               { return IN; }
[I|i][N|n][H|h][E|e][R|r][i|I][T|t][S|s] { return INHERITS; }
[L|l][E|e][T|t]                          { return LET; }
[L|l][O|o][O|o][P|p]                     { return LOOP; }
[P|p][O|o][O|o][L|l]                     { return POOL; }
[T|t][H|h][E|e][N|n]                     { return THEN; }
[W|w][H|h][I|i][L|l][E|e]                { return WHILE; }
[C|c][A|a][S|s][E|e]                     { return CASE; }
[E|e][S|s][A|a][C|c]                     { return ESAC; }
[O|o][F|f]                               { return OF; }
[N|n][E|e][W|w]                          { return NEW; }
[I|i][S|s][V|v][O|o][I|i][D|d]           { return ISVOID; }
\<\-                                     { return ASSIGN; }
\<\=                                     { return LE; }
=>                                       { return DARROW; }
--.*$                                    { }
not                                      { return NOT; }

[a-z]([A-Za-z_0-9])+     { cool_yylval.symbol = stringtable.add_string(yytext); return OBJECTID;}


\*\)                     {cool_yylval.error_msg = "Unmatched *)"; return ERROR;}

[ \t]+                    { }
\n                        {curr_lineno++;}
\[|\]|\'|!|$|%|&|~|\||#|^|\>  {cool_yylval.error_msg = yytext; return ERROR;}


%%

int add_string(char *s, bool more)
{
    size_t size = strlen(s);
    bool slashQuote = size >= 2 && s[size-2] == '\\';

    if (more){
        if(s[size-2] != '\\'){
	    reset_state();
	    cool_yylval.error_msg = "Unterminated string constant";
	    return ERROR;
	}
	size -= 2;
    } 

    if (!slashQuote)
	size--;

    for(size_t i = 0;i<size;i++){
	if (string_buf_ptr - string_buf >= MAX_STR_CONST){
	    cool_yylval.error_msg = "String constant too long";
	    reset_state();
	    return ERROR;
	}
	if (s[i] == '\\'){
	    continue;
        }
	if (s[i] == '\0'){
	    cool_yylval.error_msg = "String contains null character";
	    reset_state();
	    return ERROR;
        }
	if (s[i] == EOF){
	    printf("ok\n");
        }
	*string_buf_ptr = s[i];
	string_buf_ptr++;
    }

    if (!more && !slashQuote){
	cool_yylval.symbol = stringtable.add_string(string_buf);
	reset_state();
	return STR_CONST;
    }

    return 0;

}

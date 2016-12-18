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


extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

/**
 * Nest a comment when find one
**/
int nestComment(void);

/**
 * Add a string constant to the table. This checks for all the cases
**/
int addString(char *s);

/**
 * Add a string constant to the table. This checks for all the cases
**/
void resetState();

int commentCounter = 0;

%}

/*
 * Define names for regular expressions here.
 */

DARROW          =>
CHAR            [^\n\"]
SINGLE_TOKEN    (\+|\*|\-|\~|\/|\;|\(|\)|\{|\}|\:|\.|\@|\<|\=|\,)
ID              [a-z]([A-Za-z_0-9])*  
TYPE            [A-Z]([A-Za-z_0-9])* 
NUMBER          [0-9]+               
SPACE           [ \t\f\v\r]+
INVALID         [^a-zA-Z0-9]

%STATE          COMMENT STRING

%%

 /*
  *  Nested comments
  */
<COMMENT>[^\*\)\(\n]*  { }
<COMMENT>\*            { }
<COMMENT>\(            { }
<COMMENT>\)            { }

<COMMENT>\*\)          { int code = nestComment(); if (code) return code;}
<COMMENT><<EOF>>       { if (!eof){cool_yylval.error_msg = "EOF in comment";eof=true;return ERROR;}else{return 0;}}
\(\*                   {commentCounter++; BEGIN COMMENT;}
\*\)                   {cool_yylval.error_msg = "Unmatched *)"; return ERROR;}
<INITIAL>--[^\n]*   { }


 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
<STRING>{CHAR}*\"   { int code = addString(yytext ); if(code) return code; }
<STRING>{CHAR}*\n   { int code = addString(yytext); if (code) return code; }
<STRING><<EOF>>     { if (!eof){cool_yylval.error_msg = "EOF in string constant";eof=true;return ERROR;}else{return 0;}}
\"                  { string_buf_ptr = string_buf; BEGIN STRING; }



 /*
  *  The multiple-character operators.
  */
{SINGLE_TOKEN} { return yytext[0];}

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
t[r|R][u|U][e|E]                         { cool_yylval.boolean = true ; return BOOL_CONST;}
f[a|A][l|L][s|S][e|E]                    { cool_yylval.boolean = false; return BOOL_CONST;}
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
[n|N][O|o][T|t]                          { return NOT; }

{ID}                                     { cool_yylval.symbol = stringtable.add_string(yytext); return OBJECTID;}
{TYPE}                                   { cool_yylval.symbol = stringtable.add_string(yytext); return TYPEID;  }
{NUMBER}                                 { cool_yylval.symbol = inttable.add_string(yytext); return INT_CONST;  }

{SPACE}                                  { }
\n                                       { curr_lineno++; }
{INVALID}                                { cool_yylval.error_msg = yytext; return ERROR; }


%%
int addString(char *s)
{
    size_t size = strlen(s);
    bool more = false;

    for(size_t i = 0;i<size;i++){
	if (string_buf_ptr - string_buf >= MAX_STR_CONST){
	    cool_yylval.error_msg = "String constant too long";
	    resetState();
	    return ERROR;
	}
	if (s[i] == '\\'){
	    if (s[i+1] == 'b'){
		*string_buf_ptr = '\b';
	    } else if (s[i+1] == 'f'){
		*string_buf_ptr = '\f';
	    } else if (s[i+1] == 'n'){
		*string_buf_ptr = '\n';
            } else if (s[i+1] == '\n'){
		*string_buf_ptr = '\n';
		more = true;
	    }else if (s[i+1] == 't'){
		*string_buf_ptr = '\t';
            } else if (s[i+1] == 'v'){
		*string_buf_ptr = '\v';
	    } else if (s[i+1] == '\"'){
		*string_buf_ptr = '\"';
		more = true;
	    }else {
		*string_buf_ptr = s[i+1];
            }
	    string_buf_ptr++;
	    i++;
	    continue;
        }

	if (s[i] == '\"'){
	    cool_yylval.symbol = stringtable.add_string(string_buf);
	    resetState();
	    return STR_CONST;
	} 

        if (s[i] == '\n'){
	    resetState();
	    cool_yylval.error_msg = "Unterminated string constant";
	    return ERROR;
	} 
	*string_buf_ptr = s[i];
	string_buf_ptr++;
    }

    if (!more){
	cool_yylval.error_msg = "String contains null character";
	resetState();
	return ERROR;
    } 

    return 0;

}

int nestComment(){
    commentCounter--;

    if(commentCounter < 0){
	cool_yylval.error_msg = "Unmatched *)"; 
        return ERROR;
    } else if (commentCounter==0) { 
	BEGIN 0;
    }
    return 0;
}

void resetState(){
    memset(string_buf, 0, sizeof(string_buf));
    string_buf_ptr = string_buf;
    BEGIN 0;
}

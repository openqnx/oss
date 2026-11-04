/* A small C-like tokenizer: the scanner test_flex_scan is generated from
   this file at build time (the build machine's flex 2.6.4), and test_flex
   runs the target's flex on the same file. "test_flex_scan -" reads stdin and
   prints one line per token, then a summary; with no argument it scans a
   built-in sample and checks the counts itself. No %option noyywrap:
   yywrap() comes from libfl. */
%{
#include <stdio.h>
#include <stdlib.h>
static long sum, idents, numbers, strings, comments;
%}
%option yylineno nounput noinput
%x COMMENT
DIGIT   [0-9]
ID      [A-Za-z_][A-Za-z0-9_]*
%%
"/*"                    { BEGIN(COMMENT); comments++; }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>.|\n           { }
0[xX][0-9a-fA-F]+       { long v = strtol(yytext, NULL, 16); sum += v; numbers++; printf("%d HEX %s=%ld\n", yylineno, yytext, v); }
{DIGIT}+                { long v = atol(yytext); sum += v; numbers++; printf("%d NUM %ld\n", yylineno, v); }
if|else|while|return    { printf("%d KEYWORD %s\n", yylineno, yytext); }
{ID}                    { idents++; printf("%d IDENT %s\n", yylineno, yytext); }
\"([^"\\\n]|\\.)*\"     { strings++; printf("%d STRING %s (%d)\n", yylineno, yytext, (int)yyleng - 2); }
"=="|"<="|">="|"!="     { printf("%d OP %s\n", yylineno, yytext); }
[-+*/=<>;(){}]          { printf("%d CHAR %c\n", yylineno, yytext[0]); }
[ \t\r\n]+              { }
.                       { printf("%d ERROR '%s'\n", yylineno, yytext); }
%%
int main(int argc, char **argv)
{
    if (argc > 1 && argv[1][0] == '-') {
        yylex();
        printf("lines=%d idents=%ld numbers=%ld sum=%ld strings=%ld comments=%ld\n",
               yylineno, idents, numbers, sum, strings, comments);
        return 0;
    }
    /* Self-check: the sample test_flex also feeds through stdin. */
    YY_BUFFER_STATE b = yy_scan_string("x = 0x1F + 42; /* skip 7 \n this */\n"
                                       "if (y >= 10) return \"a\\\"b\";\n@\n");
    yylex();
    yy_delete_buffer(b);
    printf("lines=%d idents=%ld numbers=%ld sum=%ld strings=%ld comments=%ld\n",
           yylineno, idents, numbers, sum, strings, comments);
    if (yylineno == 5 && idents == 2 && numbers == 3 && sum == 83 && strings == 1 && comments == 1) {
        printf("PASS test_flex_scan\n");
        return 0;
    }
    printf("FAIL test_flex_scan\n");
    return 1;
}

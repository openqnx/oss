/* A desk calculator: the parser test_bison_calc is generated from this
   grammar at build time (bison -o test_bison_calc.c), and test_bison runs
   the target's bison on the same grammar. */
%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
static const char *in;
static long result;
int yylex(void);
void yyerror(const char *s);
%}
%define api.value.type {long}
%token NUM
%left '+' '-'
%left '*' '/' '%'
%precedence NEG
%right '^'
%%
input: expr              { result = $1; }
     ;
expr: NUM
    | expr '+' expr      { $$ = $1 + $3; }
    | expr '-' expr      { $$ = $1 - $3; }
    | expr '*' expr      { $$ = $1 * $3; }
    | expr '/' expr      { if ($3 == 0) { yyerror("division by zero"); YYERROR; } $$ = $1 / $3; }
    | expr '%' expr      { if ($3 == 0) { yyerror("division by zero"); YYERROR; } $$ = $1 % $3; }
    | '-' expr %prec NEG { $$ = -$2; }
    | expr '^' expr      { long r = 1, i; for (i = 0; i < $3; i++) r *= $1; $$ = r; }
    | '(' expr ')'       { $$ = $2; }
    ;
%%
int yylex(void)
{
  while (*in == ' ') in++;
  if (isdigit((unsigned char)*in)) { yylval = strtol(in, (char **)&in, 10); return NUM; }
  return *in ? *in++ : 0;
}
void yyerror(const char *s) { printf("error: %s\n", s); }
/* With expressions as arguments: print "<expr> = <value>" for each (what
   test_bison runs). Alone: check a few known answers itself. */
int main(int argc, char **argv)
{
  static const struct { const char *e; long v; } k[] = {
    { "2+3*4", 14 }, { "(2+3)*4", 20 }, { "2^3^2", 512 }, { "-3^2", -9 }, { "100/7-1", 13 } };
  int i, rc = 0;
  if (argc == 1) {
    printf("== bison (generated parser) ==\n");
    for (i = 0; i < (int)(sizeof k / sizeof k[0]); i++) {
      in = k[i].e;
      if (yyparse() == 0 && result == k[i].v) printf("  ok   %s = %ld\n", k[i].e, k[i].v);
      else { printf("  FAIL %s (expected %ld)\n", k[i].e, k[i].v); rc = 1; }
    }
    in = "1+";
    if (yyparse() != 0) printf("  ok   \"1+\" is a syntax error\n"); else { printf("  FAIL \"1+\" accepted\n"); rc = 1; }
    printf(rc ? "FAIL bison (generated parser)\n" : "PASS bison (generated parser)\n");
    return rc;
  }
  for (i = 1; i < argc; i++) {
    in = argv[i];
    if (yyparse() == 0) printf("%s = %ld\n", argv[i], result);
    else rc = 1;
  }
  return rc;
}

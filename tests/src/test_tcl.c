/* tcl: a C program embedding libtcl8.6. Tcl_Init finds the script library
 * (lib/tcl8.6, init.tcl), scripts evaluated through Tcl_Eval give known
 * results (expr with bignums, lists, dicts, string map, regexp, format and
 * scan, clock format in UTC, binary, TclOO, coroutines, apply, try/trap,
 * encoding), a command written in C is called from Tcl, variables are
 * shared both ways, and errors come back with their message and
 * errorCode. */
#include "common.h"
#include <tcl.h>
#include <stdlib.h>
#include <string.h>

static Tcl_Interp *I;

static void expect(const char *script, const char *want, const char *what) {
  int rc = Tcl_Eval(I, script);
  const char *got = Tcl_GetStringResult(I);
  int ok = rc == TCL_OK && !strcmp(got, want);
  if (!ok) printf("  info %s: rc %d, got \"%s\", want \"%s\"\n", what, rc, got, want);
  QT_CHECK(ok, what);
}

/* cmul a b: the product of two wide integers, an error otherwise */
static int cmul(ClientData cd, Tcl_Interp *in, int objc, Tcl_Obj *const objv[]) {
  (void)cd;
  Tcl_WideInt a, b;
  if (objc != 3) { Tcl_WrongNumArgs(in, 1, objv, "a b"); return TCL_ERROR; }
  if (Tcl_GetWideIntFromObj(in, objv[1], &a) != TCL_OK || Tcl_GetWideIntFromObj(in, objv[2], &b) != TCL_OK) return TCL_ERROR;
  Tcl_SetObjResult(in, Tcl_NewWideIntObj(a * b));
  return TCL_OK;
}

int main(int argc, char **argv) {
  (void)argc;
  QT_BEGIN("tcl");
  Tcl_FindExecutable(argv[0]);
  I = Tcl_CreateInterp();
  QT_CHECK(I != NULL, "Tcl_CreateInterp");
  int rc = Tcl_Init(I);
  if (rc != TCL_OK) printf("  info Tcl_Init: %s\n", Tcl_GetStringResult(I));
  QT_CHECK(rc == TCL_OK, "Tcl_Init finds the script library (init.tcl)");
  printf("  info patchlevel %s, library %s\n", Tcl_GetVar(I, "tcl_patchLevel", TCL_GLOBAL_ONLY),
         Tcl_GetVar(I, "tcl_library", TCL_GLOBAL_ONLY));
  expect("info patchlevel", "8.6.17", "info patchlevel 8.6.17 (the library linked)");
  expect("expr {2**100}", "1267650600228229401496703205376", "expr: bignum 2**100");
  expect("expr {7 / 2.0 + 10 % 3}", "4.5", "expr: float and integer arithmetic");
  expect("lsort -integer -decreasing {5 30 1 12}", "30 12 5 1", "lsort -integer -decreasing");
  expect("set d [dict create a 1 b 2]; dict incr d a 5; dict set d c 3; list [dict get $d a] [dict keys $d]", "6 {a b c}", "dict");
  expect("string map {foo bar 1 one} {foo1 and foo}", "barone and bar", "string map");
  expect("regexp {(\\d+)-(\\d+)} {range 10-25 end} -> a b; expr {$b - $a}", "15", "regexp with capture groups");
  expect("regsub -all {[aeiou]} {programming language} {_}", "pr_gr_mm_ng l_ng__g_", "regsub -all");
  expect("format {%05.1f|%x|%-4s|} 3.14159 255 ab", "003.1|ff|ab  |", "format");
  expect("scan {12 abc 3.5} {%d %s %f} i s f; list $i $s $f", "12 abc 3.5", "scan");
  expect("clock format 1700000000 -format {%Y-%m-%d %H:%M:%S} -gmt 1", "2023-11-14 22:13:20", "clock format -gmt (the clock library)");
  expect("clock scan {2024-02-29 12:00:00} -format {%Y-%m-%d %H:%M:%S} -gmt 1", "1709208000", "clock scan -gmt");
  expect("binary scan [binary format S2I {258 772} 16909060] S2I x y; list $x $y", "{258 772} 16909060", "binary format / scan");
  expect("oo::class create Counter { variable n; constructor {} { set n 0 }; method inc {{by 1}} { incr n $by } }; "
         "set c [Counter new]; $c inc; $c inc 4", "5", "TclOO class with a constructor and a method");
  expect("proc gen {} { yield; set i 0; while 1 { yield [incr i] } }; coroutine g gen; list [g] [g] [g]", "1 2 3", "coroutine / yield");
  expect("apply {{x y} {expr {$x * $y}}} 6 7", "42", "apply (lambda)");
  expect("try { error boom {} {QNX E1} } trap {QNX} {msg opts} { list $msg [dict get $opts -errorcode] }", "boom {QNX E1}", "try / trap on errorCode");
  expect("encoding convertto utf-8 \\u00e9", "\xc3\x83\xc2\xa9", "encoding convertto utf-8 (bytes as chars)");
  expect("string length [encoding convertto utf-8 \\u20ac]", "3", "UTF-8 encoding of U+20AC is 3 bytes");
  expect("lmap x {1 2 3 4} { if {$x % 2} continue; expr {$x * 10} }", "20 40", "lmap");
  expect("expr {[zlib decompress [zlib compress [string repeat qnx 100]]] eq [string repeat qnx 100]}", "1", "zlib compress / decompress");

  Tcl_CreateObjCommand(I, "cmul", cmul, NULL, NULL);
  expect("cmul 123456789 1000", "123456789000", "a command written in C (Tcl_CreateObjCommand)");
  rc = Tcl_Eval(I, "cmul 1 x");
  QT_CHECK(rc == TCL_ERROR && strstr(Tcl_GetStringResult(I), "expected integer but got \"x\""), "the C command reports a bad argument");
  Tcl_SetVar(I, "fromc", "41", TCL_GLOBAL_ONLY);
  expect("incr fromc", "42", "a variable set from C, used in Tcl");
  Tcl_Eval(I, "set fromtcl [string toupper qnx]");
  const char *v = Tcl_GetVar(I, "fromtcl", TCL_GLOBAL_ONLY);
  QT_CHECK(v && !strcmp(v, "QNX"), "a variable set in Tcl, read from C");
  rc = Tcl_Eval(I, "expr {1 / 0}");
  const char *ec = Tcl_GetVar(I, "errorCode", TCL_GLOBAL_ONLY);
  QT_CHECK(rc == TCL_ERROR && !strcmp(Tcl_GetStringResult(I), "divide by zero") && ec && !strncmp(ec, "ARITH DIVZERO", 13),
           "expr {1 / 0}: error \"divide by zero\", errorCode ARITH DIVZERO");
  Tcl_DeleteInterp(I);
  Tcl_Finalize();
  QT_END();
}

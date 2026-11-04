#!/bin/sh
# tcl: tclsh8.6 (and the tclsh link) runs scripts. Known answers: a script
# file with arguments (argv, file I/O, glob, exec of a QNX program, a
# pipe), and the packages bundled with Tcl 8.6: sqlite3 (an in-memory
# database: create, insert, aggregate query, a Tcl function called from
# SQL), Thread (a worker thread computes and sends back a result), itcl
# (a class with inheritance) and TDBC's core package.
echo "== tclsh =="
P=${QNX_PORT_PREFIX:-/usr/local}
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails + 1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1"; else bad "$1"; echo "  info want: $3"; echo "  info got:  $2"; fi; }
[ -x "$P/bin/tclsh8.6" ] || { echo "  FAIL tclsh8.6 installed"; echo "FAIL tclsh"; exit 1; }
chk "tclsh (link) and tclsh8.6: patchlevel" "$(echo 'puts [info patchlevel]' | "$P/bin/tclsh" 2>&1) $(echo 'puts [info patchlevel]' | "$P/bin/tclsh8.6" 2>&1)" "8.6.17 8.6.17"
T=${TMPDIR:-/tmp}/tclsh-test.$$; mkdir -p "$T"
cat > "$T/t.tcl" <<'TCL'
set out {}
lappend out [llength $argv] [lindex $argv 1]
set f [open [file join [lindex $argv 0] data.txt] w]
foreach w {delta alpha charlie bravo} { puts $f $w }
close $f
set f [open [file join [lindex $argv 0] data.txt]]
set words [split [string trim [read $f]] \n]
close $f
lappend out [join [lsort $words] ,]
lappend out [llength [glob -directory [lindex $argv 0] *.txt]]
lappend out [string trim [exec echo from-exec]]
set p [open "|sort -r" r+]; puts $p "a\nc\nb"; flush $p; chan close $p write
lappend out [join [split [string trim [read $p]] \n] ""]; close $p
puts [join $out |]
TCL
chk "tclsh runs a script file: argv, file I/O, glob, exec, a pipe" "$("$P/bin/tclsh8.6" "$T/t.tcl" "$T" second 2>&1)" "2|second|alpha,bravo,charlie,delta|1|from-exec|cba"
out=$("$P/bin/tclsh8.6" 2>&1 <<'TCL'
package require sqlite3
sqlite3 db :memory:
db function triple {apply {x {expr {$x * 3}}}}
db eval {create table t(name text, n integer)}
foreach {a b} {x 1 y 2 z 3 x 4} { db eval {insert into t values($a, $b)} }
puts [db eval {select name, sum(n) from t group by name order by name}]
puts [db eval {select triple(max(n)) from t}]
puts [db onecolumn {select sqlite_version()}]
db close
TCL
)
chk "sqlite3 package: in-memory db, group by, a Tcl function from SQL" "$(echo "$out" | head -n 2)" "x 5 y 2 z 3
12"
echo "  info sqlite $(echo "$out" | sed -n 3p)"
out=$("$P/bin/tclsh8.6" 2>&1 <<'TCL'
package require Thread
set tid [thread::create { proc sq {x} { expr {$x * $x} }; thread::wait }]
set r [thread::send $tid {sq 12}]
thread::release $tid
puts "$r [llength [thread::names]]"
TCL
)
chk "Thread package: a worker thread computes 12*12" "$out" "144 1"
out=$("$P/bin/tclsh8.6" 2>&1 <<'TCL'
package require itcl
itcl::class Shape { public variable name; constructor {n} { set name $n }; method area {} { return 0 }; method describe {} { return "$name:[area]" } }
itcl::class Rect { inherit Shape; variable w; variable h; constructor {a b} { Shape::constructor rect } { set w $a; set h $b }; method area {} { expr {$w * $h} } }
Rect r1 3 4
puts [r1 describe]
TCL
)
chk "itcl package: a class with inheritance and a virtual method" "$out" "rect:12"
out=$(echo 'puts [package require tdbc]' | "$P/bin/tclsh8.6" 2>&1)
chk "tdbc package loads" "$out" "1.1.12"
rm -rf "$T"
[ $fails -eq 0 ] && { echo "PASS tclsh"; exit 0; }
echo "FAIL tclsh ($fails)"; exit 1

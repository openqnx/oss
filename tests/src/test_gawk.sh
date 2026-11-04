#!/bin/sh
# gawk: real programs with known results -- field sums, associative arrays,
# gawk's own gensub/asort/strftime/bit functions, printf formatting, a
# loadable extension (@load "ordchr") from /usr/local/lib/gawk and an
# @include from the installed awk library (share/awk).
echo "== gawk =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
G=/usr/local/bin/gawk; [ -x "$G" ] || G=gawk
T=$(mktemp -d /tmp/qnxport-gawk.XXXXXX) || { echo "FAIL gawk (mktemp)"; exit 1; }
cd "$T" || exit 1
check() {  # check <what> <expected> <got>
  if [ "$3" = "$2" ]; then ok "$1"; else bad "$1: expected '$2', got '$3'"; fi
}
v=$("$G" --version 2>&1 | head -n 1)
echo "  info $v"
case "$v" in "GNU Awk 5.3.2"*) ok "gawk --version: GNU Awk 5.3.2" ;; *) bad "gawk --version: $v" ;; esac

printf 'alice 10 3\nbob 20 4\ncarol 30 5\nbob 5 1\n' > data.txt
check "sum of column 2" "65" "$("$G" '{s+=$2} END{print s}' data.txt)"
check "NR and NF" "4 3" "$("$G" 'END{print NR, NF}' data.txt)"
check "associative array, sorted traversal" "alice=10 bob=25 carol=30" \
  "$("$G" '{t[$1]+=$2} END{PROCINFO["sorted_in"]="@ind_str_asc"; for(k in t) o=o (o?" ":"") k "=" t[k]; print o}' data.txt)"
check "-F and OFS" "a-b-c" "$(echo 'a:b:c' | "$G" -F: -v OFS=- '{$1=$1; print}')"
check "printf formatting" "[  42|3.142|ff|0X1F|abc  ]" \
  "$("$G" 'BEGIN{printf "[%4d|%.3f|%x|%#X|%-5s]\n", 42, 3.14159265, 255, 31, "abc"}')"
check "gsub count and result" "3 x-y-z-" "$(echo 'x.y.z.' | "$G" '{n=gsub(/\./,"-"); print n, $0}')"
check "gensub with back-references" "world hello" "$(echo 'hello world' | "$G" '{print gensub(/([a-z]+) ([a-z]+)/, "\\2 \\1", "g")}')"
check "split and asort" "1 2 5 9" "$("$G" 'BEGIN{n=split("9,2,5,1",a,","); asort(a); print a[1], a[2], a[3], a[4]}')"
check "toupper/substr/index/length" "QNX 3 4" "$("$G" 'BEGIN{s="the qnx os"; print toupper(substr(s,5,3)), index(s,"e"), length("abcd")}')"
check "bit functions and strtonum" "12 14 255" "$("$G" 'BEGIN{print and(14,13), or(12,2)+0, strtonum("0xff")}')"
check "strftime in UTC" "2009-02-13 23:31:30" "$("$G" 'BEGIN{print strftime("%Y-%m-%d %H:%M:%S", 1234567890, 1)}')"
check "regex match() with array" "2023 07" "$(echo 'date: 2023-07-14' | "$G" '{if (match($0, /([0-9]{4})-([0-9]{2})/, m)) print m[1], m[2]}')"
check "user-defined recursive function" "3628800" "$("$G" 'function f(n){return n<2?1:n*f(n-1)} BEGIN{print f(10)}')"
check "getline from command" "hello" "$("$G" 'BEGIN{"echo hello" | getline x; print x}')"
check "integer arithmetic 2^53" "9007199254740992" "$("$G" 'BEGIN{printf "%d\n", 2^53}')"
printf 'k1 v1\nk2 v2\n' > map.txt; printf 'k2\nk1\nk3\n' > keys.txt
check "two-file join (NR==FNR)" "k2:v2 k1:v1 k3:-" \
  "$("$G" 'NR==FNR{m[$1]=$2; next} {o=o (o?" ":"") $1 ":" (($1 in m)?m[$1]:"-")} END{print o}' map.txt keys.txt)"
check "output redirection to a file" "b a" "$("$G" 'BEGIN{print "b" > "out.txt"; print "a" > "out.txt"; close("out.txt"); while((getline l < "out.txt")>0) o=o (o?" ":"") l; print o}')"
# Dynamic extension: ordchr.so from AWKLIBPATH's default (/usr/local/lib/gawk).
check "@load \"ordchr\" (ord/chr)" "65 z" "$("$G" '@load "ordchr"; BEGIN{print ord("A"), chr(122)}' 2>&1)"
check "@load \"filefuncs\" (stat size)" "17" "$(printf '0123456789abcdef\n' > s.bin; "$G" '@load "filefuncs"; BEGIN{stat("s.bin", st); print st["size"]}' 2>&1)"
# The awk library from AWKPATH's default (/usr/local/share/awk).
check "@include \"join.awk\"" "a+b+c" "$("$G" '@include "join.awk"; BEGIN{n=split("a b c",x," "); print join(x,1,n,"+")}' 2>&1)"
# awk is installed as a link to gawk.
[ -x /usr/local/bin/awk ] && check "/usr/local/bin/awk runs gawk" "6" "$(/usr/local/bin/awk 'BEGIN{print 1+2+3}')"
cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS gawk"; exit 0; }
echo "FAIL gawk ($fail check(s) failed)"; exit 1

#!/bin/sh
# gettext: xgettext extracts messages from C source, msgfmt compiles a German
# catalog, gettext/ngettext look translations (and plural forms) up through
# libintl, msgunfmt decompiles the .mo again, msgmerge/msgcat/msggrep work on
# catalogs and envsubst substitutes variables. Translations are ASCII so the
# result does not depend on the target's (C-only) locale charset.
echo "== gettext =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
check() { if [ "$3" = "$2" ]; then ok "$1"; else bad "$1: expected '$2', got '$3'"; fi; }
B=/usr/local/bin
T=$(mktemp -d /tmp/qnxport-gettext.XXXXXX) || { echo "FAIL gettext (mktemp)"; exit 1; }
cd "$T" || exit 1
v=$("$B/msgfmt" --version 2>&1 | head -n 1); echo "  info $v"
case "$v" in *"0.22.5"*) ok "msgfmt --version: 0.22.5" ;; *) bad "msgfmt --version: $v" ;; esac

cat > hello.c <<'EOF'
#include <libintl.h>
#define _(s) gettext(s)
int main(int n) {
  puts(_("Hello, world"));
  puts(_("Good morning"));
  printf(ngettext("%d file", "%d files", n), n);
  return 0;
}
EOF
"$B/xgettext" --keyword=_ --from-code=UTF-8 -o qt.pot hello.c 2>xg.err
if [ -f qt.pot ] && grep -q '^msgid "Hello, world"' qt.pot && grep -q '^msgid "Good morning"' qt.pot \
   && grep -q '^msgid_plural "%d files"' qt.pot; then ok "xgettext: 3 messages extracted (one plural)"
else bad "xgettext: $(cat xg.err)"; fi

cat > de.po <<'EOF'
msgid ""
msgstr ""
"Project-Id-Version: qnx-port-test 1.0\n"
"PO-Revision-Date: 2026-01-01 00:00+0000\n"
"Last-Translator: QNX port test <test@example.invalid>\n"
"Language-Team: German <de@example.invalid>\n"
"Language: de\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"
"Plural-Forms: nplurals=2; plural=(n != 1);\n"

msgid "Hello, world"
msgstr "Hallo, Welt"

msgid "Good morning"
msgstr "Guten Morgen"

msgid "%d file"
msgid_plural "%d files"
msgstr[0] "%d Datei"
msgstr[1] "%d Dateien"
EOF
mkdir -p loc/de/LC_MESSAGES
# --statistics goes to stderr together with any --check diagnostics: the
# header is complete, so a clean run prints exactly the statistics line.
# The output is in a nested directory: msgfmt writes it through gnulib's
# supersede/canonicalize, which readlink()s every directory on the way.
"$B/msgfmt" --check --statistics -o loc/de/LC_MESSAGES/qt.mo de.po 2>mf.err; rc=$?
st=$(cat mf.err); echo "  info msgfmt (rc=$rc): $st"
[ $rc -eq 0 ] && [ -s loc/de/LC_MESSAGES/qt.mo ] && ok "msgfmt: .mo written into loc/de/LC_MESSAGES ($(wc -c < loc/de/LC_MESSAGES/qt.mo) bytes)" || bad "msgfmt -o loc/de/LC_MESSAGES/qt.mo: rc=$rc $st"
check "msgfmt --check --statistics: no warnings, 3 translated" "3 translated messages." "$st"
"$B/msgfmt" -o here.mo de.po 2>mf2.err && [ -s here.mo ] && cmp -s here.mo loc/de/LC_MESSAGES/qt.mo \
  && ok "msgfmt -o into the current directory: same .mo" || bad "msgfmt -o here.mo: $(cat mf2.err)"
# GNU .mo magic 0x950412de, in either byte order
m=$(od -An -tx1 -N4 loc/de/LC_MESSAGES/qt.mo | tr -d ' \n')
case "$m" in de120495|950412de) ok "mo magic $m" ;; *) bad "mo magic: $m" ;; esac

# libintl looks LC_ALL / LANGUAGE up in the environment itself (QNX's
# setlocale knows only "C").
L=de_run
de_run() { ( TEXTDOMAINDIR="$T/loc" LC_ALL=de_DE.UTF-8 LANG=de_DE.UTF-8 LANGUAGE=de; export TEXTDOMAINDIR LC_ALL LANG LANGUAGE; "$@" ); }
check "gettext: translated" "Hallo, Welt" "$($L "$B/gettext" -d qt "Hello, world" 2>&1)"
check "gettext: second message" "Guten Morgen" "$($L "$B/gettext" -d qt "Good morning" 2>&1)"
check "gettext: untranslated msgid passes through" "Good night" "$($L "$B/gettext" -d qt "Good night" 2>&1)"
check "gettext with LC_ALL=C stays English" "Hello, world" \
  "$(TEXTDOMAINDIR="$T/loc" LC_ALL=C LANGUAGE=de "$B/gettext" -d qt "Hello, world" 2>&1)"
check "ngettext n=1: singular" "%d Datei" "$($L "$B/ngettext" -d qt "%d file" "%d files" 1 2>&1)"
check "ngettext n=5: plural" "%d Dateien" "$($L "$B/ngettext" -d qt "%d file" "%d files" 5 2>&1)"
check "gettext -s" "Hallo, Welt" "$($L "$B/gettext" -d qt -s "Hello, world" 2>&1)"

"$B/msgunfmt" -o back.po loc/de/LC_MESSAGES/qt.mo 2>un.err
if grep -q '^msgstr "Guten Morgen"' back.po && grep -q '^msgstr\[1\] "%d Dateien"' back.po; then ok "msgunfmt: translations recovered"
else bad "msgunfmt: $(cat un.err)"; fi

# msgmerge the catalog against the template; msggrep picks one message.
"$B/msgmerge" -q -o merged.po de.po qt.pot 2>mm.err && grep -q '^msgstr "Hallo, Welt"' merged.po \
  && ok "msgmerge: translations kept against the .pot" || bad "msgmerge: $(cat mm.err)"
n=$("$B/msggrep" --msgid -e "morning" de.po 2>&1 | grep -c '^msgid "Good morning"')
check "msggrep: selects one message" "1" "$n"
c=$("$B/msgcat" --no-wrap de.po back.po 2>&1 | grep -c '^msgid "Hello, world"')
check "msgcat: duplicates merged" "1" "$c"

check "envsubst" "user=qnx home=/root" "$(echo 'user=$U home=$H' | U=qnx H=/root "$B/envsubst" 2>&1)"
check "envsubst with a variable list" 'a=1 b=$B' "$(echo 'a=$A b=$B' | A=1 B=2 "$B/envsubst" '$A' 2>&1)"
cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS gettext"; exit 0; }
echo "FAIL gettext ($fail check(s) failed)"; exit 1

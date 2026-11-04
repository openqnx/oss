#!/bin/sh
# vala: valac on the target, generating C (the target has no C compiler,
# so the C is checked, not compiled). Known answers: valac --version; a
# program with an interface, a class with construct properties and a
# signal, an enum, a generic function with an owned delegate, lambdas
# capturing locals, nullable strings and GLib.Regex translated with -C,
# --header and --vapi to exactly the C, header and VAPI that the same valac
# 0.56.18 writes on the build machine (MD5 below: that C was compiled
# there with GLib and printed "area=48 seen=2 total=7 color=DEMO_COLOR_BLUE
# maybe=none" and "15"), with the expected GObject boilerplate in it;
# --pkg gio-2.0 found in the installed VAPIs; and a type error reported
# with its position and exit status 1.
echo "== vala =="
P=${QNX_PORT_PREFIX:-/usr/local}
V=$P/bin/valac
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails + 1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1"; else bad "$1"; echo "  info want: $3"; echo "  info got:  $2"; fi; }
md5() { md5sum < "$1" | cut -d' ' -f1; }
[ -x "$V" ] || { echo "  FAIL valac installed ($V)"; echo "FAIL vala"; exit 1; }
chk "valac --version" "$("$V" --version 2>&1)" "Vala 0.56.18"
T=${TMPDIR:-/tmp}/vala-test.$$; mkdir -p "$T"; cd "$T" || exit 1
cat > demo.vala <<'VALA'
namespace Demo {
  public interface Shape : Object {
    public abstract double area ();
  }
  public class Rect : Object, Shape {
    public double w { get; construct; }
    public double h { get; construct; }
    public signal void resized (double factor);
    public Rect (double w, double h) { Object (w: w, h: h); }
    public double area () { return w * h; }
    public void scale (double f) { _w *= f; _h *= f; resized (f); }
  }
  public enum Color { RED, GREEN, BLUE }
  int sum<T> (GenericArray<T> items, owned MapFunc<int, T> f) {
    int s = 0;
    items.foreach ((x) => { s += f (x); });
    return s;
  }
}
public delegate R MapFunc<R, T> (T x);
int main (string[] args) {
  var r = new Demo.Rect (3, 4);
  double seen = 0;
  r.resized.connect ((f) => { seen = f; });
  r.scale (2);
  var names = new GenericArray<string> ();
  names.add ("alpha"); names.add ("be");
  int total = Demo.sum<string> (names, (s) => s.length);
  string? maybe = args.length > 5 ? "x" : null;
  stdout.printf ("area=%g seen=%g total=%d color=%s maybe=%s\n", r.area (), seen, total,
                 Demo.Color.BLUE.to_string (), maybe ?? "none");
  try {
    var re = new Regex ("(\\d+)-(\\d+)");
    MatchInfo mi;
    if (re.match ("x 10-25", 0, out mi)) stdout.printf ("%d\n", int.parse (mi.fetch (2)) - int.parse (mi.fetch (1)));
  } catch (RegexError e) { return 1; }
  return 0;
}
VALA
out=$("$V" -C --header=demo.h --vapi=demo.vapi demo.vala 2>&1); rc=$?
[ $rc -eq 0 ] && [ -z "$out" ] && ok "valac -C --header --vapi demo.vala (rc 0, no diagnostics)" || { bad "valac -C (rc $rc)"; echo "$out" | head -10; }
chk "demo.c is the build machine's valac 0.56.18 output (MD5)" "$(md5 demo.c)" "ec344784582f9492acc1c85e5b86e2a8"
chk "demo.h likewise" "$(md5 demo.h)" "3d142e028c137b07b71e31e5a813975e"
chk "demo.vapi likewise" "$(md5 demo.vapi)" "5affd97fb9929d6651990e42e54b6efe"
for s in "demo_rect_get_type" "g_signal_new (\"resized\"" "g_signal_emit (self, demo_rect_signals[DEMO_RECT_RESIZED_SIGNAL], 0, f)" \
         "demo_shape_area (DemoShape* self)" "DEMO_COLOR_BLUE" "g_ptr_array_foreach (items" "g_regex_new (" "_vala_main (gchar** args" \
         "g_object_new (object_type, \"w\", w, \"h\", h, NULL)"; do
  grep -qF "$s" demo.c && ok "demo.c has: $s" || bad "demo.c has: $s"
done
grep -q "^VALA_EXTERN gdouble demo_shape_area (DemoShape\* self);" demo.h && grep -q "^VALA_EXTERN DemoRect\* demo_rect_new (gdouble w," demo.h \
  && ok "demo.h declares demo_shape_area and demo_rect_new" || bad "demo.h declarations"
grep -q "public class Rect : GLib.Object, Demo.Shape" demo.vapi && ok "demo.vapi describes class Rect" || bad "demo.vapi class Rect"
printf 'void main () {\n  var f = File.new_for_path ("/tmp/a.txt");\n  print ("%%s\\n", f.get_basename ());\n}\n' > gio.vala
out=$("$V" -C --pkg gio-2.0 gio.vala 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q "g_file_new_for_path" gio.c && grep -q "g_file_get_basename" gio.c && ok "--pkg gio-2.0 (installed gio-2.0.vapi): g_file_new_for_path" \
  || { bad "--pkg gio-2.0 (rc $rc)"; echo "$out" | head -5; }
printf 'void main () {\n  int x = "s";\n}\n' > bad.vala
out=$("$V" -C bad.vala 2>&1); rc=$?
if [ $rc -eq 1 ] && echo "$out" | grep -qF "bad.vala:2.7-2.7: error: Assignment: Cannot convert from \`string' to \`int'" \
   && echo "$out" | grep -q "Compilation failed: 1 error(s), 0 warning(s)"; then ok "a type error: position, message, exit 1"
else bad "a type error (rc $rc)"; echo "$out"; fi
cd / && rm -rf "$T"
[ $fails -eq 0 ] && { echo "PASS vala"; exit 0; }
echo "FAIL vala ($fails)"; exit 1

#!/bin/sh
# ruby: the interpreter on the target. Known answers:
# - miniruby (the build's interpreter, no built-in extensions) and ruby run the same script of
#   core classes -- Integer bignums, Rational/Complex, String/Symbol, Array/
#   Hash/Enumerable (lazy), Struct, Comparable, pattern matching, Fiber,
#   Thread + Queue, exceptions, Time.at(...).utc, frozen string literals --
#   and print the same output; deep recursion in a thread raises
#   SystemStackError;
# - C extensions of the standard library: json, zlib (crc32, deflate and
#   inflate), openssl (SHA-256 and HMAC of known inputs, a random key round
#   trip with AES), psych (YAML), stringio, date, digest, socket (a TCP echo
#   on 127.0.0.1), etc (Etc.uname), fiddle (strlen from libc), io/console,
#   bigdecimal (a bundled gem with a C extension), ripper;
# - the gems shipped with it: gem list shows default and bundled gems,
#   minitest runs a passing and a failing test, rake runs a Rakefile, erb
#   renders a template, and a gem built on the target from a gemspec is
#   installed from the local file and loaded.
echo "== ruby =="
P=${QNX_PORT_PREFIX:-/usr/local}
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails + 1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1"; else bad "$1"; echo "  info want: $3"; echo "  info got:  $2"; fi; }
R=$P/bin/ruby
[ -x "$R" ] || { echo "  FAIL ruby installed ($R)"; echo "FAIL ruby"; exit 1; }
T=${TMPDIR:-/tmp}/ruby-test.$$; rm -rf "$T"; mkdir -p "$T"; cd "$T" || exit 1
HOME=$T; export HOME; unset RUBYOPT RUBYLIB GEM_HOME GEM_PATH
echo "  info $("$R" -v 2>&1)"
chk "ruby -v" "$("$R" -e 'print RUBY_VERSION, " ", RUBY_PLATFORM' 2>&1)" "3.4.12 $("$R" -e 'print RUBY_PLATFORM')"
case "$("$R" -e 'print RUBY_PLATFORM')" in *-nto-qnx*) ok "RUBY_PLATFORM names QNX ($("$R" -e 'print RUBY_PLATFORM'))" ;; *) bad "RUBY_PLATFORM names QNX" ;; esac

cat > core.rb <<'RB'
# frozen_string_literal: true
out = []
out << (2**100).to_s << (2**100 % 97).to_s
out << (Rational(1, 3) + Rational(1, 6)).to_s << (Complex(1, 2) * Complex(3, -1)).to_s
out << "Hello, QNX".sub(/QNX/) { |m| m.downcase }.reverse
out << %w[pear fig apple banana].sort_by { |w| [w.size, w] }.join(",")
h = Hash.new(0); "the quick the lazy the".split.each { |w| h[w] += 1 }
out << h.sort_by { |k, v| [-v, k] }.map { |k, v| "#{k}=#{v}" }.join(" ")
out << (1..Float::INFINITY).lazy.select(&:even?).map { _1 * _1 }.first(4).inspect
Pt = Struct.new(:x, :y) do
  include Comparable
  def <=>(o) = (x**2 + y**2) <=> (o.x**2 + o.y**2)
end
out << [Pt.new(3, 4), Pt.new(1, 1), Pt.new(0, 2)].sort.map { |p| "(#{p.x},#{p.y})" }.join
r = case {name: "qnx", ver: [8, 0]}
    in {name: String => n, ver: [8, minor]} then "#{n} 8.#{minor}"
    end
out << r
f = Fiber.new { |a| b = Fiber.yield(a + 1); b * 2 }
out << "#{f.resume(1)} #{f.resume(10)}"
q = Queue.new
th = 4.times.map { |i| Thread.new { q << i * i } }; th.each(&:join)
out << 4.times.map { q.pop }.sum.to_s
begin
  Integer("x1")
rescue ArgumentError => e
  out << e.class.name
end
out << Time.at(1_700_000_000).utc.strftime("%Y-%m-%d %H:%M:%S")
s = "lit"; begin; s << "x"; rescue FrozenError; out << "frozen"; end
out << [1, [2, [3, [4]]]].flatten.each_slice(2).map(&:sum).inspect
puts out.join("|")
RB
want='1267650600228229401496703205376|16|1/2|5+5i|xnq ,olleH|fig,pear,apple,banana|the=3 lazy=1 quick=1|[4, 16, 36, 64]|(1,1)(0,2)(3,4)|qnx 8.0|2 20|14|ArgumentError|2023-11-14 22:13:20|frozen|[3, 7]'
chk "ruby runs the core-classes script" "$("$R" core.rb 2>&1)" "$want"
if [ -x "$P/bin/miniruby" ]; then
  chk "miniruby runs the same script, same output" "$("$P/bin/miniruby" core.rb 2>&1)" "$want"
else bad "miniruby installed ($P/bin/miniruby)"; fi
# a thread's stack is known to the VM: deep recursion in a thread raises
# SystemStackError instead of crashing (QNX needed a patch for this)
chk "stack overflow in a thread raises SystemStackError" "$("$R" -e 'def f(n) = n == 0 ? 0 : 1 + f(n - 1); t = Thread.new { begin; f(10**7); rescue SystemStackError => e; e.class; end }; print f(5000), " ", t.value' 2>&1)" "5000 SystemStackError"

cat > ext.rb <<'RB'
out = []
require "json"
d = JSON.parse('{"a":[1,2,{"b":null}],"c":"é"}')
out << JSON.generate(d) << d["c"].bytesize.to_s
require "zlib"
data = "qnx " * 1000
z = Zlib::Deflate.deflate(data)
out << Zlib.crc32("The quick brown fox jumps over the lazy dog").to_s(16) << (Zlib::Inflate.inflate(z) == data && z.size < 100).to_s
require "openssl"
out << OpenSSL::Digest::SHA256.hexdigest("abc")
out << OpenSSL::HMAC.hexdigest("SHA256", "key", "The quick brown fox jumps over the lazy dog")
c = OpenSSL::Cipher.new("aes-256-cbc").encrypt; key = c.random_key; iv = c.random_iv
ct = c.update("secret message") + c.final
dc = OpenSSL::Cipher.new("aes-256-cbc").decrypt; dc.key = key; dc.iv = iv
out << (dc.update(ct) + dc.final)
require "yaml"
y = YAML.safe_load("a: 1\nb: [x, y]\nc: {d: true}\n")
out << y.inspect << YAML.dump({"k" => [1, 2]}).lines.map(&:strip).join(";")
require "stringio"; io = StringIO.new; io.puts "x"; io.print 1, 2; out << io.string.inspect
require "date"; out << (Date.new(2024, 3, 1) - 1).to_s << Date.new(2024, 12, 25).strftime("%A")
require "digest"; out << Digest::MD5.hexdigest("abc")
require "socket"
srv = TCPServer.new("127.0.0.1", 0); port = srv.addr[1]
t = Thread.new { c = srv.accept; c.write(c.gets.upcase); c.close }
s = TCPSocket.new("127.0.0.1", port); s.puts "echo over tcp"; out << s.read.strip; s.close; t.join
require "etc"; out << Etc.uname[:sysname]
require "fiddle"
libc = Fiddle.dlopen(nil)
strlen = Fiddle::Function.new(libc["strlen"], [Fiddle::TYPE_VOIDP], Fiddle::TYPE_SIZE_T)
out << strlen.call("fiddle!").to_s
require "io/console"; out << IO.respond_to?(:console).to_s
require "bigdecimal"; out << (BigDecimal("0.1") + BigDecimal("0.2")).to_s
require "ripper"; out << Ripper.sexp("1 + 2")[1][0][0].to_s
puts out.join("|")
RB
want='{"a":[1,2,{"b":null}],"c":"é"}|2|414fa339|true|ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad|f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8|secret message|{"a" => 1, "b" => ["x", "y"], "c" => {"d" => true}}|---;k:;- 1;- 2|"x\n12"|2024-02-29|Wednesday|900150983cd24fb0d6963f7d28e17f72|ECHO OVER TCP|QNX|7|true|0.3e0|binary'
chk "C extensions: json zlib openssl psych stringio date digest socket etc fiddle io/console bigdecimal ripper" "$("$R" ext.rb 2>&1)" "$want"

out=$("$R" -S gem list 2>&1)
for g in "json (" "minitest (" "rake (" "bigdecimal (" "bundler (" "psych ("; do
  echo "$out" | grep -q "^$g" && ok "gem list: $g...)" || bad "gem list: $g...)"
done
cat > t_test.rb <<'RB'
require "minitest/autorun"
class TQ < Minitest::Test
  def test_pass; assert_equal 4, 2 + 2; end
  def test_fail; assert_equal 5, 2 + 2; end
end
RB
out=$("$R" t_test.rb --seed 1 2>&1); rc=$?
if [ $rc -ne 0 ] && echo "$out" | grep -q "2 runs, 2 assertions, 1 failures, 0 errors"; then ok "minitest: 2 runs, 1 failure (rc $rc)"
else bad "minitest (rc $rc)"; echo "$out" | tail -5; fi
cat > Rakefile <<'RB'
task :a do puts "a" end
task :b => :a do puts "b" end
task default: :b
RB
chk "rake runs a Rakefile with a dependency" "$("$P/bin/rake" 2>&1 | tr '\n' ' ')" "a b "
chk "erb renders a template" "$(echo '<% 3.times do |i| %><%= i * 2 %>,<% end %>' | "$P/bin/erb" 2>&1)" "0,2,4,"

mkdir -p mygem/lib
cat > mygem/lib/qnxgreet.rb <<'RB'
module QnxGreet; def self.hi(n) = "hi #{n}"; end
RB
cat > mygem/qnxgreet.gemspec <<'RB'
Gem::Specification.new do |s|
  s.name = "qnxgreet"; s.version = "1.2.3"; s.summary = "test gem"; s.authors = ["test"]
  s.files = ["lib/qnxgreet.rb"]; s.license = "MIT"
end
RB
( cd mygem && "$R" -S gem build qnxgreet.gemspec >/dev/null 2>&1 )
[ -f mygem/qnxgreet-1.2.3.gem ] && ok "gem build: qnxgreet-1.2.3.gem" || bad "gem build"
out=$("$R" -S gem install --local --no-document --install-dir "$T/gems" mygem/qnxgreet-1.2.3.gem 2>&1); rc=$?
[ $rc -eq 0 ] && echo "$out" | grep -q "Successfully installed qnxgreet-1.2.3" && ok "gem install --local from the file" || { bad "gem install --local (rc $rc)"; echo "$out" | tail -3; }
chk "the installed gem loads" "$(GEM_PATH=$T/gems "$R" -e 'gem "qnxgreet", "1.2.3"; require "qnxgreet"; print QnxGreet.hi("qnx")' 2>&1)" "hi qnx"
cd / && rm -rf "$T"
[ $fails -eq 0 ] && { echo "PASS ruby"; exit 0; }
echo "FAIL ruby ($fails)"; exit 1

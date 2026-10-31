#!/bin/sh
# luv: the libuv bindings Neovim's Lua API sits on (lib/libluv.so, built for
# LuaJIT), loaded into the ported LuaJIT interpreter and driving a libuv loop:
# timers firing in timeout order, a TCP echo exchange on 127.0.0.1 inside the
# one loop, synchronous and asynchronous fs calls in /tmp, a child process's
# exit status and piped output. A guard timer stops the loop if anything hangs.
echo "== luv =="
P=${QNX_PORT_PREFIX:-/usr/local}
LJ=$P/bin/luajit
[ -x "$LJ" ] || LJ=$(command -v luajit 2>/dev/null)
if [ -z "$LJ" ] || [ ! -x "$LJ" ]; then
  echo "  FAIL luajit interpreter found ($P/bin/luajit)"; echo "FAIL luv"; exit 1
fi
echo "  ok   luajit interpreter found ($LJ)"
LUV_LIB=$P/lib/libluv.so
[ -f "$LUV_LIB" ] || LUV_LIB=$P/lib/libluv.so.1
LUV_TMP=${LUV_TMP:-/tmp/qnxport-luv.$$}; export LUV_LIB LUV_TMP
out=$("$LJ" - 2>&1 <<'LUAEOF'
local fails = 0
local function check(c, what) if c then print("  ok   " .. what) else print("  FAIL " .. what); fails = fails + 1 end end
local open, err = package.loadlib(os.getenv("LUV_LIB"), "luaopen_luv")
check(open ~= nil, "package.loadlib libluv.so luaopen_luv")
if not open then print("  info " .. tostring(err)); os.exit(1) end
local uv = open()
check(type(uv) == "table" and type(uv.run) == "function", "luv module table")
print("  info luv on libuv " .. uv.version_string() .. ", " .. jit.version)
check(uv.version() >= 0x012c00, "libuv >= 1.44 (" .. uv.version_string() .. ")")

-- guard: stop the loop if something never completes
local guard = uv.new_timer()
local timed_out = false
guard:start(15000, 0, function() timed_out = true; uv.stop() end)

-- timers: fire in order of their timeouts, a repeating one counts to 3
local order = {}
local t1, t2, t3 = uv.new_timer(), uv.new_timer(), uv.new_timer()
t1:start(60, 0, function() order[#order + 1] = "b"; t1:close() end)
t2:start(10, 0, function() order[#order + 1] = "a"; t2:close() end)
local ticks = 0
t3:start(5, 5, function() ticks = ticks + 1; if ticks == 3 then t3:stop(); t3:close() end end)

-- TCP echo over loopback: the server upper-cases what it receives
local server = uv.new_tcp()
check(server:bind("127.0.0.1", 0) == 0, "tcp bind 127.0.0.1:0")
local addr = server:getsockname()
check(addr and addr.port and addr.port > 0, "tcp listening port assigned")
print("  info server on " .. tostring(addr and addr.ip) .. ":" .. tostring(addr and addr.port))
server:listen(8, function(e)
  local c = uv.new_tcp(); server:accept(c)
  local acc = ""
  c:read_start(function(re, chunk)
    if chunk then
      acc = acc .. chunk
      if acc:find("\n") then c:write(acc:upper()); c:shutdown(function() c:close() end) end
    else c:close() end
  end)
end)
local reply, connected = "", false
local client = uv.new_tcp()
client:connect("127.0.0.1", addr.port, function(e)
  connected = (e == nil)
  if e then print("  info connect: " .. tostring(e)); client:close(); server:close(); return end
  client:write("ping from luv\n")
  client:read_start(function(re, chunk)
    if chunk then reply = reply .. chunk else client:close(); server:close() end
  end)
end)

-- fs: synchronous write / read back, asynchronous stat
local path = os.getenv("LUV_TMP") or "/tmp/qnxport-luv"
local fd = uv.fs_open(path, "w", 420)
check(fd ~= nil, "fs_open for writing in /tmp")
local payload = string.rep("0123456789abcdef", 256)   -- 4096 bytes
if fd then check(uv.fs_write(fd, payload, 0) == #payload, "fs_write 4096 bytes"); uv.fs_close(fd) end
fd = uv.fs_open(path, "r", 0)
local back = fd and uv.fs_read(fd, 8192, 0)
if fd then uv.fs_close(fd) end
check(back == payload, "fs_read returns the same 4096 bytes")
local stat_size, stat_done
uv.fs_stat(path, function(e, st) stat_size = st and st.size; stat_done = true end)

-- a child process: exit status and its stdout through a pipe
local pipe = uv.new_pipe(false)
local child_out, child_code = "", nil
local h, pid = uv.spawn("/bin/sh", { args = { "-c", "echo child-says-hi; exit 3" }, stdio = { nil, pipe, nil } },
  function(code, sig) child_code = code end)
check(h ~= nil, "uv.spawn /bin/sh")
if not h then print("  info spawn: " .. tostring(pid)); pipe:close()
else
  pipe:read_start(function(re, chunk) if chunk then child_out = child_out .. chunk else pipe:close() end end)
end

local idle_runs = 0
local idle = uv.new_idle()
idle:start(function() idle_runs = idle_runs + 1; if idle_runs >= 2 then idle:stop(); idle:close() end end)

-- once everything else has closed, only the guard remains: close it so run() returns
local check_h = uv.new_check()
check_h:start(function()
  if #order == 2 and ticks == 3 and (reply ~= "" or not connected) and stat_done and (child_code or not h) and child_out ~= nil
     and (not h or pipe:is_closing()) then
    check_h:stop(); check_h:close(); guard:stop(); guard:close()
  end
end)
local t0 = uv.hrtime()
uv.run()
if h and not h:is_closing() then h:close() end
uv.run("nowait")
check(not timed_out, "loop finished before the 15 s guard")
check(uv.hrtime() > t0, "hrtime is monotonic")
check(table.concat(order) == "ab", "timers fired in timeout order (10 ms before 60 ms)")
check(ticks == 3, "repeating timer fired 3 times")
check(idle_runs >= 2, "idle handle ran")
check(connected, "tcp client connected over loopback")
check(reply == "PING FROM LUV\n", "tcp echo reply upper-cased by the server")
check(stat_size == 4096, "async fs_stat reports 4096 bytes")
check(child_out == "child-says-hi\n", "child stdout read through a pipe")
check(child_code == 3, "child exit status 3")
uv.fs_unlink(path)
check(uv.fs_stat(path) == nil, "fs_unlink removed the file")
print("  ok   lua script ran to completion")
os.exit(fails == 0 and 0 or 1)
LUAEOF
)
rc=$?
rm -f "$LUV_TMP"
echo "$out"
if [ $rc -eq 0 ] && echo "$out" | grep -q "ran to completion" && ! echo "$out" | grep -q "^  FAIL"; then
  echo "PASS luv"; exit 0
fi
echo "FAIL luv (luajit rc=$rc)"; exit 1

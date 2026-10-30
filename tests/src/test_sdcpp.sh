#!/bin/sh
# stable-diffusion.cpp: no model ships with it (they are gigabytes), so this
# checks what runs without one -- sd-cli starts (its libraries found through
# its run path), reports the commit it was built from, and refuses a missing
# model cleanly with the loader's message. Image generation is not tested.
echo "== stable-diffusion.cpp =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
SD=$(command -v sd-cli || echo /usr/local/opt/stable-diffusion.cpp/bin/sd-cli)
v=$("$SD" --version 2>&1); echo "  info $(echo "$v" | head -n 1)"
echo "$v" | grep -q "commit 7948df8" && ok "sd-cli --version: the commit built (7948df8)" || bad "sd-cli --version"
o=$("$SD" -m /nonexistent.safetensors -p test -o /tmp/sd-$$.png 2>&1); rc=$?
[ $rc -ne 0 ] && [ ! -f /tmp/sd-$$.png ] && ok "missing model refused (rc=$rc)" || bad "missing model refused"
echo "$o" | grep -q "init model loader from file failed" && ok "the loader reports the missing file" || bad "loader message"
[ $fail -eq 0 ] && { echo "PASS stable-diffusion.cpp"; exit 0; }
echo "FAIL stable-diffusion.cpp ($fail check(s) failed)"; exit 1

#!/bin/sh
# python3-sounddevice on the ported PortAudio (ALSA host API on QNX's
# libasound), without a sound device (the test VMs have none): the library
# loads (patch 0001: by soname, find_library() finds nothing on QNX), the
# version is PortAudio 19.7.0, the ALSA host API is listed, the device list
# and defaults answer, a missing device raises PortAudioError/ValueError, and
# check_output_settings() on an absent device is refused. On a machine with
# an output device, 0.2 s of silence is played with a blocking stream.
echo "== python3-sounddevice =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-sounddevice.XXXXXX") || { echo "FAIL python3-sounddevice (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
cat > t.py <<'P'
import sys
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
def info(s): print("  info " + s)
import sounddevice as sd
v = sd.get_portaudio_version()
info("sounddevice %s, %s" % (sd.__version__, v[1]))
check(v[0] == 0x130700 and v[1].startswith("PortAudio V19.7.0"), "PortAudio 19.7.0")
apis = sd.query_hostapis()
check(any(a["name"] == "ALSA" for a in apis), "the ALSA host API")
devs = sd.query_devices()
info("%d device(s)" % len(devs))
check(isinstance(devs, sd.DeviceList), "query_devices() answers")
try:
    sd.query_devices(len(devs) + 5)
    check(False, "a missing device index is an error")
except (sd.PortAudioError, ValueError, IndexError):
    check(True, "a missing device index is an error")
outs = [i for i, d in enumerate(devs) if d["max_output_channels"] > 0]
if outs:
    import array
    with sd.RawOutputStream(samplerate=48000, channels=1, dtype="int16", device=outs[0]) as s:
        s.write(array.array("h", [0] * 9600).tobytes())
    check(True, "0.2 s of silence on output device %d" % outs[0])
else:
    try:
        sd.check_output_settings(device=0, channels=1, samplerate=48000)
        check(False, "check_output_settings without a device is refused")
    except (sd.PortAudioError, ValueError):
        check(True, "check_output_settings without a device is refused")
sys.exit(1 if fails else 0)
P
"$PY" t.py; rc=$?
[ $rc -eq 0 ] && { echo "PASS python3-sounddevice"; exit 0; }
echo "FAIL python3-sounddevice (rc=$rc)"; exit 1
